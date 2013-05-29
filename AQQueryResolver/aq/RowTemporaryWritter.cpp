#include "RowTemporaryWritter.h"
#include <aq/Utilities.h>

namespace aq
{
  
  RowTemporaryWritter::RowTemporaryWritter(unsigned int _tableId, const char * _path, unsigned int _packetSize)
    : 
    RowWritter(_path),
    tableId(_tableId),
    path(_path),
    packetSize(_packetSize)
  {
  }
  
  RowTemporaryWritter::~RowTemporaryWritter()
  {
    std::for_each(this->columnsWritter.begin(), this->columnsWritter.end(), [] (boost::shared_ptr<ColumnTemporaryWritter> ctw) {
      if (ctw->file)
      {
        fclose(ctw->file); 
        ctw->file = 0;
      }
    });
  }

  int RowTemporaryWritter::process(Row& row)
  {
    if (this->columns.size() == 0)
    {
      uint16_t columnId = 1;
      for (Row::row_t::const_reverse_iterator it = row.computedRow.rbegin(); it != row.computedRow.rend(); ++it)
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
        std::string type_str = columnTypeToStr(type);

        Column::Ptr column(new Column(name, ID, size, type));
        column->Temporary = true;
        this->columns.push_back(column);

        boost::shared_ptr<ColumnTemporaryWritter> ctw(new ColumnTemporaryWritter);
        ctw->column = column;
        ctw->filename = getTemporaryFileName( tableId, columnId, 0, type_str.c_str(), size);
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
      uint64_t count = row.count;
      for (Row::row_t::const_reverse_iterator it = row.computedRow.rbegin(); it != row.computedRow.rend(); ++it)
      {
        if (!(*it).displayed)
          continue;
        assert((*it).item != NULL);

        switch (this->columnsWritter[c]->column->Type)
        {
        case ColumnType::COL_TYPE_BIG_INT:
        case ColumnType::COL_TYPE_DATE1:
        case ColumnType::COL_TYPE_DATE2:
        case ColumnType::COL_TYPE_DATE3:
        case ColumnType::COL_TYPE_DATE4:
          {
            int64_t value = static_cast<int64_t>((*it).item->numval);
            fwrite(&value, sizeof(int64_t), 1, this->columnsWritter[c]->file);
          }
          break;
        case ColumnType::COL_TYPE_DOUBLE:
          {
            double value = (*it).item->numval;
            fwrite(&value, sizeof(double), 1, this->columnsWritter[c]->file);
          }
          break;
        case ColumnType::COL_TYPE_INT:
          {
            int32_t value = static_cast<int32_t>((*it).item->numval);
            fwrite(&value, sizeof(int32_t), 1, this->columnsWritter[c]->file);
          }
          break;
        case ColumnType::COL_TYPE_VARCHAR:
          {
            assert(strlen((*it).item->strval.c_str()) <= this->columnsWritter[c]->column->Size);
            char * value = new char[this->columnsWritter[c]->column->Size + 1];
            memset(value, 0, this->columnsWritter[c]->column->Size + 1);
            strcpy(value, (*it).item->strval.c_str());
            fwrite(value, sizeof(char) * this->columnsWritter[c]->column->Size, 1, this->columnsWritter[c]->file);
            free(value);
          }
          break;
        }
        
        if ((this->totalCount % this->packetSize) == 0)
        {
          unsigned int packet = this->totalCount / this->packetSize;
          unsigned int ID = c + 1;
          unsigned int size = (*it).size; 
          aq::ColumnType type = (*it).type;
          std::string type_str = columnTypeToStr(type);
          boost::shared_ptr<ColumnTemporaryWritter> ctw = this->columnsWritter[c];
          ctw->filename = getTemporaryFileName( tableId, ID, packet, type_str.c_str(), size);
          ctw->filename = path + "/" + ctw->filename;
          fclose(ctw->file);
          ctw->file = fopen(ctw->filename.c_str(), "wb");
        }

        this->columnsWritter[c]->nbEl += 1;
        c++;
      }
    }

    if (row.flush)
    {
      std::for_each(this->columnsWritter.begin(), this->columnsWritter.end(), [] (boost::shared_ptr<ColumnTemporaryWritter> ctw) { 
        fclose(ctw->file); 
        ctw->file = 0;
      });
    }

    return 0;
  }

}