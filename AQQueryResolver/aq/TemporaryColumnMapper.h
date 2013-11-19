#ifndef __TEMPORARY_COLUMN_MAPPER_H__
#define __TEMPORARY_COLUMN_MAPPER_H__

#include "ColumnMapper_Intf.h"
#include <aq/FileMapper.h>
#include <vector>

namespace aq
{

template <typename T>
class TemporaryColumnMapper : public ColumnMapper_Intf<T>
{
public:
	TemporaryColumnMapper(const char * _path, size_t _tableId, size_t _columnId, aq::ColumnType itemType, size_t _itemSize, size_t _packetSize);
	~TemporaryColumnMapper();
	int loadValue(size_t index,T& value);
	int setValue(size_t index, T value);
  int append(T value);
private:
  uint64_t nbRemap;
  const std::string path;
	size_t tableId;
  size_t columnId;
  size_t itemSize;
  aq::ColumnType itemType;
  size_t packetSize;
  size_t currentPacket;
	boost::shared_ptr<aq::FileMapper> tmpMapper;
	std::vector<boost::shared_ptr<aq::FileMapper> > tmpMappers;
  std::vector<std::string> temporaryFiles;
};

template <typename T>
TemporaryColumnMapper<T>::TemporaryColumnMapper(const char * _path, size_t _tableId, size_t _columnId, aq::ColumnType _itemType, size_t _itemSize, size_t _packetSize)
  : 
  nbRemap(0),
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
	case COL_TYPE_DATE:
    strcpy(type_str, "LON"); 
    break;
  }
  sprintf( prefix, "B001TMP%.4uC%.4u%s%.4uP", tableId, columnId, type_str, itemSize );
	getFileNames(path.c_str(), this->temporaryFiles, prefix );
  assert(!this->temporaryFiles.empty());
	this->tmpMapper.reset(new aq::FileMapper(this->temporaryFiles[this->currentPacket].c_str()));
  tmpMappers.resize(temporaryFiles.size());
}

template <typename T>
TemporaryColumnMapper<T>::~TemporaryColumnMapper()
{
}

template <typename T>
int TemporaryColumnMapper<T>::loadValue(size_t offset, T& value)
{
  size_t packet = offset / packetSize;
  offset = offset % packetSize;
  if (packet != currentPacket)
  {
    ++nbRemap;
    currentPacket = packet;
    if (tmpMappers[currentPacket])
    {
      this->tmpMapper = tmpMappers[currentPacket];
    }
    else
    {
      boost::shared_ptr<aq::FileMapper> mapper(new aq::FileMapper(this->temporaryFiles[this->currentPacket].c_str()));
      tmpMappers[currentPacket] = this->tmpMapper;
      this->tmpMapper = mapper;
    }
  }

  return this->tmpMapper->read(value, offset * itemSize, itemSize);
}

template <typename T>
int TemporaryColumnMapper<T>::setValue(size_t, T)
{
  throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "set value not implemented for temporary column mapper");
  return 0;
}

template <typename T>
int TemporaryColumnMapper<T>::append(T)
{
  throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "append value not implemented for temporary column mapper");
  return 0;
}

}

#endif