#include "RowTemporaryWritter.h"
#include <aq/Utilities.h>
#include <aq/Database.h>
#include <cstring>

namespace aq
{
  
  RowTemporaryWritter::RowTemporaryWritter(unsigned int _tableId, const char * _path, unsigned int _packetSize)
    : 
    path(_path),
    tableId(_tableId),
    packetSize(_packetSize),
    totalCount(0)
  {
  }
  
  RowTemporaryWritter::RowTemporaryWritter(const RowTemporaryWritter& o)
    : 
    path(o.path),
    tableId(o.tableId),
    packetSize(o.packetSize),
    totalCount(o.totalCount)
  {
  }
  
  RowTemporaryWritter::~RowTemporaryWritter()
  {
    for (auto& ctw : this->columnsWritter) 
    {
      if (ctw->file)
      {
        fclose(ctw->file); 
        ctw->file = 0;
      }
    }
  }
  
  int RowTemporaryWritter::process(std::vector<Row>& rows)
  {
    for (auto& row : rows) 
    { 
      this->process(row); 
    }
    return 0;
  }
  
  int RowTemporaryWritter::process(Row& row)
  {
    if (this->columnsWritter.size() == 0)
    {
      uint16_t columnId = 1;
      for (Row::row_t::const_reverse_iterator it = row.computedRow.rbegin(); it != row.computedRow.rend(); ++it)
      {
        if (!(*it).displayed)
          continue;
        
        std::string name = (*it).columnName;
        if ((*it).tableName != "")
        {
          // FIXME
          // name = (*it).tableName + "." + name;
        }
        unsigned int ID = columnId;
        unsigned int size = (*it).size; 
        aq::ColumnType type = (*it).type;
        std::string type_str = columnTypeToStr(type);

        Column::Ptr column(new Column(name, ID, size, type));
        column->Temporary = true;
        column->setTableName((*it).tableName);
        this->columns.push_back(column);

        boost::shared_ptr<ColumnTemporaryWritter> ctw(new ColumnTemporaryWritter);
        ctw->column = column;
        ctw->filename = aq::Database::getTemporaryFileName(tableId, columnId, 0, type_str.c_str(), size);
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
      for (Row::row_t::const_reverse_iterator it = row.computedRow.rbegin(); it != row.computedRow.rend(); ++it)
      {
        if (!(*it).displayed)
          continue;

        switch (this->columnsWritter[c]->column->getType())
        {
        case ColumnType::COL_TYPE_BIG_INT:
        case ColumnType::COL_TYPE_DATE:
          {
            int64_t value = boost::get<aq::ColumnItem<int64_t> >((*it).item).getValue();
            fwrite(&value, sizeof(int64_t), 1, this->columnsWritter[c]->file);
          }
          break;
        case ColumnType::COL_TYPE_DOUBLE:
          {
            double value = boost::get<aq::ColumnItem<double> >((*it).item).getValue();
            fwrite(&value, sizeof(double), 1, this->columnsWritter[c]->file);
          }
          break;
        case ColumnType::COL_TYPE_INT:
          {
            int32_t value = boost::get<aq::ColumnItem<int32_t> >((*it).item).getValue();
            fwrite(&value, sizeof(int32_t), 1, this->columnsWritter[c]->file);
          }
          break;
        case ColumnType::COL_TYPE_VARCHAR:
          {
            //assert(strlen((*it).item->strval.c_str()) <= this->columnsWritter[c]->column->Size);
            //char * value = new char[this->columnsWritter[c]->column->Size + 1];
            //memset(value, 0, this->columnsWritter[c]->column->Size + 1);
            //strcpy(value, (*it).item->strval.c_str());
            const char * value = boost::get<aq::ColumnItem<char*> >((*it).item).getValue();
            fwrite(value, sizeof(char) * this->columnsWritter[c]->column->getSize(), 1, this->columnsWritter[c]->file);
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
          ctw->filename = aq::Database::getTemporaryFileName( tableId, ID, packet, type_str.c_str(), size);
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
      for (auto& ctw : this->columnsWritter) 
      { 
        fclose(ctw->file); 
        ctw->file = 0;
      }
    }

    return 0;
  }

}
