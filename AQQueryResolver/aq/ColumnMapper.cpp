#include "ColumnMapper.h"
#include <aq/Logger.h>
#include <aq/Exceptions.h>
#include <aq/Utilities.h>

using namespace aq;

ColumnMapper::ColumnMapper(const char * _path, size_t _tableId, size_t _columnId, aq::ColumnType _type, size_t _size, size_t _packetSize)
	: nbRemap(0),
		tableId(_tableId),
		columnId(_columnId),
		type(_type),
    size(_size),
		currentPart(0),
		packetSize(_packetSize),
		path(_path)
{
	std::string prmFilename = getPrmFileName(path.c_str(), tableId, columnId, currentPart);
	std::string thesaurusFilename = getThesaurusFileName(path.c_str(), tableId, columnId, currentPart);
	this->prmMapper.reset(new aq::FileMapper(prmFilename.c_str()));
	this->thesaurusMapper.reset(new aq::FileMapper(thesaurusFilename.c_str()));
}

ColumnMapper::~ColumnMapper()
{
	aq::Logger::getInstance().log(AQ_DEBUG, "%u remaping\n", nbRemap);
}

ColumnItem::Ptr ColumnMapper::loadValue(size_t index)
{
	//std::map<size_t, ColumnItem::Ptr>::const_iterator itPrm = this->prm.find(index);
	//if (itPrm != this->prm.end())
	//{
	//	c = *(itPrm->second);
	//}

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

			this->prmMapper.reset(new aq::FileMapper(prmFilename.c_str()));
			this->thesaurusMapper.reset(new aq::FileMapper(thesaurusFilename.c_str()));
			this->prmMappers.insert(std::make_pair(currentPart, this->prmMapper));
			this->thesaurusMappers.insert(std::make_pair(currentPart, this->thesaurusMapper));
		}
		else
		{
			this->prmMapper = this->prmMappers[currentPart];
			this->thesaurusMapper = this->thesaurusMappers[currentPart];
		}
	}
	uint32_t offset = 0;
	this->prmMapper->read(&offset, i * sizeof(uint32_t), sizeof(uint32_t));
	ColumnItem::Ptr value(new ColumnItem);
	switch (type)
	{
	case aq::COL_TYPE_VARCHAR:
		{ 
			char val[128];
			this->thesaurusMapper->read(val, offset * size, size);
			value->strval = val;
		}
		break;
	case aq::COL_TYPE_DOUBLE:
		{
			this->thesaurusMapper->read(&value->numval, offset * sizeof(double), sizeof(double));
		}
		break;
	case aq::COL_TYPE_INT:
		{
			int32_t val;
			this->thesaurusMapper->read(&val, offset * sizeof(int32_t), sizeof(int32_t));
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
			this->thesaurusMapper->read(&val, offset * sizeof(int64_t), sizeof(int64_t));
			value->numval = static_cast<double>(val);
		}
		break;
	default:
		break;
	}
	
	//if (this->prm.find(index) == this->prm.end())
	//{
	//	this->prm[index] = value;
	//	this->thesaurus[value].push_back(index);
	//}
	
	return value;
}

const std::vector<size_t>& ColumnMapper::getSimilarIndex(size_t index) const
{
	std::map<size_t, ColumnItem::Ptr>::const_iterator it = this->prm.find(index);
	if (it == this->prm.end())
	{
		// todo : raise exception
	}
	std::map<ColumnItem::Ptr, std::vector<size_t>, struct column_cmp_t >::const_iterator it2 = this->thesaurus.find(it->second);
	if (it2 == this->thesaurus.end())
	{
		// todo : raise exception
	}
	return it2->second;
}