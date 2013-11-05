#ifndef __TEMPORARY_COLUMN_MAPPER_H__
#define __TEMPORARY_COLUMN_MAPPER_H__

#include "ColumnMapper_Intf.h"
#include <aq/FileMapper.h>
#include <vector>

namespace aq
{

class TemporaryColumnMapper : public ColumnMapper_Intf
{
public:
	TemporaryColumnMapper(const char * _path, size_t _tableId, size_t _columnId, aq::ColumnType itemType, size_t _itemSize, size_t _packetSize);
	~TemporaryColumnMapper();
	int loadValue(size_t index, ColumnItem& value);
	int setValue(size_t index, ColumnItem& value);
  int append(ColumnItem& value);
  virtual const aq::ColumnType getType() const { return itemType; }
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

}

#endif