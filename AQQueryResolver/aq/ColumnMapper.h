#ifndef __AQ_COLUMN_MAPPER_H__
#define __AQ_COLUMN_MAPPER_H__

#include "ColumnItem.h"
#include <aq/FileMapper.h>
#include <aq/DBTypes.h>
#include <set>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace aq
{
	
class ColumnMapper
{
public:
	typedef boost::shared_ptr<ColumnMapper> Ptr;

	ColumnMapper(const char * path, size_t tableId, size_t columnId, aq::ColumnType type, size_t _size, size_t _packetSize);
	~ColumnMapper();
	ColumnItem::Ptr loadValue(size_t index);
	const std::vector<size_t>& getSimilarIndex(size_t index) const;
	const aq::ColumnType getType() const { return this->type; }
private:
	size_t nbRemap;
	size_t tableId;
	size_t columnId;
	aq::ColumnType type;
  size_t size;
	size_t currentPart;
	size_t packetSize;
	const std::string path;
	boost::shared_ptr<aq::FileMapper> prmMapper;
	boost::shared_ptr<aq::FileMapper> thesaurusMapper;
	std::map<size_t, boost::shared_ptr<aq::FileMapper> > prmMappers;
	std::map<size_t, boost::shared_ptr<aq::FileMapper> > thesaurusMappers;
	std::map<size_t, ColumnItem::Ptr> prm;
	std::map<ColumnItem::Ptr, std::vector<size_t>, struct column_cmp_t > thesaurus;
};

}

#endif