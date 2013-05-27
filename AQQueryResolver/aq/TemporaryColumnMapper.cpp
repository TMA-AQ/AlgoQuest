#include "TemporaryColumnMapper.h"
#include <aq/Utilities.h>
#include <boost/lexical_cast.hpp>

namespace aq
{

TemporaryColumnMapper::TemporaryColumnMapper(const char * _path, size_t _tableId, size_t _columnId, aq::ColumnType _itemType, size_t _itemSize, size_t _packetSize)
  : 
  path(_path),
  tableId(_tableId),
  columnId(_columnId),
  itemSize(_itemSize),
  itemType(_itemType),
  packetSize(_packetSize),
  currentPacket(0)
{
  char prefix[128];
  char type_str[128];
  switch (itemType)
  {
  case COL_TYPE_VARCHAR: strcpy(type_str, "CHA"); break;
	case COL_TYPE_INT: strcpy(type_str, "INT"); break;
	case COL_TYPE_DOUBLE: strcpy(type_str, "DOU"); break;
	case COL_TYPE_BIG_INT:
	case COL_TYPE_DATE1:
	case COL_TYPE_DATE2:
	case COL_TYPE_DATE3:
	case COL_TYPE_DATE4: strcpy(type_str, "LON"); break;
  }
  sprintf( prefix, "B001TMP%.4uC%.4u%s%.4uP", tableId, columnId, type_str, itemSize );
	getFileNames(path.c_str(), this->temporaryFiles, prefix );
  assert(!this->temporaryFiles.empty());
	this->tmpMapper.reset(new aq::FileMapper(this->temporaryFiles[this->currentPacket].c_str()));
}

TemporaryColumnMapper::~TemporaryColumnMapper()
{
}

ColumnItem::Ptr TemporaryColumnMapper::loadValue(size_t offset)
{
  size_t packet = offset / packetSize;
  offset = offset % packetSize;
  if (packet != currentPacket)
  {
    currentPacket = packet;
    this->tmpMapper.reset(new aq::FileMapper(this->temporaryFiles[this->currentPacket].c_str()));
  }

  int rc = 0;
	ColumnItem::Ptr value(new ColumnItem);
  switch (itemType)
	{
	case aq::COL_TYPE_VARCHAR:
		{ 
			char val[128];
			rc = this->tmpMapper->read(val, offset * itemSize, itemSize);
			value->strval = val;
		}
		break;
	case aq::COL_TYPE_DOUBLE:
		{
			rc = this->tmpMapper->read(&value->numval, offset * sizeof(double), sizeof(double));
		}
		break;
	case aq::COL_TYPE_INT:
		{
			int32_t val;
			rc = this->tmpMapper->read(&val, offset * sizeof(int32_t), sizeof(int32_t));
			value->numval = static_cast<double>(val);
		}
		break;
	case aq::COL_TYPE_BIG_INT:
	case aq::COL_TYPE_DATE1:
	case aq::COL_TYPE_DATE2:
	case aq::COL_TYPE_DATE3:
	case aq::COL_TYPE_DATE4:
		{
			int64_t val;
			rc = this->tmpMapper->read(&val, offset * sizeof(int64_t), sizeof(int64_t));
			value->numval = static_cast<double>(val);
		}
		break;
	default:
		break;
	}

  if (rc == -1)
  {
    value = NULL;
  }

  return value;
}

}