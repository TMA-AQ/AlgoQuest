#include "RowTemporaryWritter.h"
#include <aq/Utilities.h>

namespace aq
{
  
  RowTemporaryWritter::RowTemporaryWritter(unsigned int _tableId, const char * _path)
    : 
    RowWritter(_path),
    tableId(_tableId),
    path(_path)
  {
  }
  
  RowTemporaryWritter::~RowTemporaryWritter()
  {
  }

  int RowTemporaryWritter::process(Row& row)
  {
    if (this->columns.size() == 0)
    {
      uint16_t columnId = 1;
      for (row_t::const_reverse_iterator it = row.row.rbegin(); it != row.row.rend(); ++it)
      {
        if (!(*it).displayed)
          continue;
        
        std::string name = (*it).columnName;
        if ((*it).tableName != "")
        {
          name = (*it).tableName + "." + name;
        }
        unsigned int ID = columnId;
        unsigned int size = (*it).size; 
        aq::ColumnType type = (*it).type;
        
        std::string type_str;
        switch(type)
        {
        case ColumnType::COL_TYPE_INT: type_str = "INT"; size = 4; break;
        case ColumnType::COL_TYPE_DOUBLE: type_str = "DOU"; size = 8; break;
        case ColumnType::COL_TYPE_VARCHAR: type_str = "CHA"; break;
        case ColumnType::COL_TYPE_BIG_INT:
        case ColumnType::COL_TYPE_DATE1:
        case ColumnType::COL_TYPE_DATE2:
        case ColumnType::COL_TYPE_DATE3:
        case ColumnType::COL_TYPE_DATE4: type_str = "LON"; size = 8; break;
        }
        
        Column::Ptr column(new Column(name, ID, size, type));
        this->columns.push_back(column);

        boost::shared_ptr<ColumnTemporaryWritter> ctw(new ColumnTemporaryWritter);
        ctw->column = column;
        ctw->filename = getThesaurusTemporaryFileName( tableId, columnId, 0, type_str.c_str(), size);
        ctw->filename = path + "/" + ctw->filename;
        ctw->file = fopen(ctw->filename.c_str(), "wb");
        this->columnsWritter.push_back(ctw);
        
        columnId++;
      }
    }

    if (row.completed)
    {
      this->totalCount += 1;
      uint16_t c = 0;
      uint64_t count = static_cast<uint64_t>((*row.row.begin()).item->numval);
      for (row_t::const_reverse_iterator it = row.row.rbegin(); it != row.row.rend(); ++it)
      {
        if (!(*it).displayed)
          continue;
        assert((*it).item != NULL);

        fwrite(&(*it).item->numval, sizeof(double), 1, this->columnsWritter[c]->file);
        this->columnsWritter[c]->nbEl += 1;
        c++;
      }
    }

    if (row.flush)
    {
      std::for_each(this->columnsWritter.begin(), this->columnsWritter.end(), [] (boost::shared_ptr<ColumnTemporaryWritter> ctw) { 
        fclose(ctw->file); 
      });
    }

    return 0;
  }

}