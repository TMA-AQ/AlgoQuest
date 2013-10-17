#ifndef __AQ_COLUMN_MAPPER_H__
#define __AQ_COLUMN_MAPPER_H__

#include "ColumnMapper_Intf.h"
#include <aq/Utilities.h>
#include <aq/Logger.h>
#include <aq/DBTypes.h>
#include <aq/Exceptions.h>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

namespace aq
{
  
// ---------------------------------------------------------------------------------
template <typename T, class M>
class ColumnMapper : public ColumnMapper_Intf
{
public:
	ColumnMapper(const char * path, size_t tableId, size_t columnId, size_t _size, size_t _packetSize);
	~ColumnMapper();
	int loadValue(size_t index, ColumnItem& value);
	const std::vector<size_t>& getSimilarIndex(size_t index) const;
	const aq::ColumnType getType() const { return type_conversion<T>::type; }
  static void fill_item(ColumnItem& item, T * value, size_t size);
private:
	static const aq::ColumnType type;
	size_t nbRemap;
	size_t tableId;
	size_t columnId;
  size_t size;
	size_t currentPart;
	size_t packetSize;
	const std::string path;
	boost::shared_ptr<M> prmMapper;
	boost::shared_ptr<M> thesaurusMapper;
	std::map<size_t, boost::shared_ptr<M> > prmMappers;
	std::map<size_t, boost::shared_ptr<M> > thesaurusMappers;
};

template <typename T, class M>
ColumnMapper<T,M>::ColumnMapper(const char * _path, size_t _tableId, size_t _columnId, size_t _size, size_t _packetSize)
	: nbRemap(0),
		tableId(_tableId),
		columnId(_columnId),
    size(_size),
		currentPart(0),
		packetSize(_packetSize),
		path(_path)
{
	std::string prmFilename = getPrmFileName(path.c_str(), tableId, columnId, currentPart);
	std::string thesaurusFilename = getThesaurusFileName(path.c_str(), tableId, columnId, currentPart);
	this->prmMapper.reset(new M(prmFilename.c_str()));
	this->thesaurusMapper.reset(new M(thesaurusFilename.c_str()));
}

template <typename T, class M>
ColumnMapper<T,M>::~ColumnMapper()
{
	aq::Logger::getInstance().log(AQ_DEBUG, "%u remaping\n", nbRemap);
}

template <typename T, class M>
int ColumnMapper<T,M>::loadValue(size_t index, ColumnItem& value)
{
	size_t part = index / packetSize;
	size_t i = index % packetSize;
	if (currentPart != part)
	{
		++nbRemap;
		currentPart = part;

		if (this->prmMappers.find(currentPart) == this->prmMappers.end())
		{
			std::string prmFilename = getPrmFileName(path.c_str(), tableId, columnId, currentPart);
			std::string thesaurusFilename = getThesaurusFileName(path.c_str(), tableId, columnId, currentPart);

			aq::Logger::getInstance().log(AQ_DEBUG, "open %s\n", prmFilename.c_str());
			aq::Logger::getInstance().log(AQ_DEBUG, "open %s\n", thesaurusFilename.c_str());

			boost::filesystem::path p1(prmFilename);
			boost::filesystem::path p2(thesaurusFilename);
			if (!boost::filesystem::exists(p1) || !boost::filesystem::exists(p2))
			{
        throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "");
			}

			this->prmMapper.reset(new M(prmFilename.c_str()));
			this->thesaurusMapper.reset(new M(thesaurusFilename.c_str()));
			this->prmMappers.insert(std::make_pair(currentPart, this->prmMapper));
			this->thesaurusMappers.insert(std::make_pair(currentPart, this->thesaurusMapper));
		}
		else
		{
			this->prmMapper = this->prmMappers[currentPart];
			this->thesaurusMapper = this->thesaurusMappers[currentPart];
		}
	}

  int rc = 0;
	uint32_t offset = 0;
	if ((rc = this->prmMapper->read(&offset, i * sizeof(uint32_t), sizeof(uint32_t))) != -1)
  {
    T * val = new T[size];
    if ((rc = this->thesaurusMapper->read(val, offset * size * sizeof(T), size * sizeof(T))) == 0)
    {
      aq::fill_item<T>(value, val, this->size);
    }
  }
  return rc;
}

}

#endif
