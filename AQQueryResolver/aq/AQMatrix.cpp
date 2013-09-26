#include "AQMatrix.h"
#include "Base.h"
#include <aq/Logger.h>
#include <aq/Utilities.h>
#include <aq/Exceptions.h>
#include <iostream>
#include <set>
#include <list>
#include <aq/FileMapper.h>
#include <boost/scoped_array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

using namespace aq;
 
#define STR_BIG_BUF_SIZE 128

namespace
{
  
	struct inner_column_cmp_t
	{
	public:
		inner_column_cmp_t(const std::vector<size_t>& lessThanColumn)
			: m_lessThanColumn(lessThanColumn)
		{
		}
		bool operator()(size_t idx1, size_t idx2)
		{
			return m_lessThanColumn[idx1] < m_lessThanColumn[idx2];
		}
	private:
		const std::vector<size_t>& m_lessThanColumn;
	};

	struct row_cmp_t
	{
	public:
		row_cmp_t(std::vector<ColumnMapper_Intf::Ptr>& _columnMapper)
			: columnMapper(_columnMapper),
				row1(_columnMapper.size()),
				row2(_columnMapper.size())
		{
		}
		bool operator()(size_t idx1, size_t idx2)
		{
			// fill row to compare
			size_t pos = 0;
			ColumnItem item;
			std::for_each(columnMapper.begin(), columnMapper.end(), [&] (ColumnMapper_Intf::Ptr& c) {
				c->loadValue(idx1, *row1[pos]);
				c->loadValue(idx2, *row2[pos]);
				++pos;
			});
			// and compare
			for (pos = 0; pos < row1.size(); ++pos)
			{
				if (ColumnItem::lessThan(*row1[pos], *row2[pos])) return true;
				else if (ColumnItem::lessThan(*row2[pos], *row1[pos])) return false;
			}
			return false;
		}
	private:
		std::vector<ColumnMapper_Intf::Ptr>& columnMapper;
		std::vector<ColumnItem::Ptr> row1;
		std::vector<ColumnItem::Ptr> row2;
	};

	struct index_holder_t
	{
		size_t index;
		row_cmp_t& cmp;
	};

	struct index_holder_cmp_t
	{
		bool operator()(const index_holder_t& ih1, const index_holder_t& ih2)
		{
			return (ih1.cmp)(ih1.index, ih2.index);
		}
	};

}

uint64_t AQMatrix::uid_generator = 0;

AQMatrix::AQMatrix(const TProjectSettings& _settings, const Base& _baseDesc)
	: settings(_settings),
    baseDesc(_baseDesc),
		totalCount(0),
		nbRows(0),
		hasCount(false)
{
  uid = ++AQMatrix::uid_generator;
}

AQMatrix::AQMatrix(const AQMatrix& source)
	: settings(source.settings),
    baseDesc(source.baseDesc),
		totalCount(source.totalCount),
		nbRows(source.nbRows),
		hasCount(source.hasCount)
{
}

AQMatrix::~AQMatrix()
{
}

AQMatrix& AQMatrix::operator=(const AQMatrix& source)
{
	if (this != &source)
	{
		totalCount = source.totalCount;
		nbRows = source.nbRows;
		hasCount = source.hasCount;
	}
	return *this;
}

void AQMatrix::simulate(size_t rows, const std::vector<long long>& tableIDs)
{
	this->hasCount = true;
	this->totalCount = rows;
	this->nbRows = rows;
	this->matrix.resize(tableIDs.size());

  for ( size_t idx = 0; idx < tableIDs.size(); ++idx )
    this->matrix[idx].table_id = static_cast<size_t>( tableIDs[idx] );

	for (std::vector<column_t>::iterator it = this->matrix.begin(); it != this->matrix.end(); ++it)
    for (size_t idx = 0; idx < rows; ++idx)
      (*it).indexes.push_back(rand() % this->baseDesc.getTable((*it).table_id)->TotalCount);
	this->count.resize(rows, 1);

  this->groupByIndex.push_back(std::make_pair(rows, rows));
}

void AQMatrix::clear()
{
  matrix.clear();
	count.clear();
  groupByIndex.clear();
	totalCount = 0;
	nbRows = 0;
	hasCount = false;
}

void AQMatrix::write(const char * filePath)
{
  FILE * fd = fopen(filePath, "wb");
  uint64_t value = this->matrix.size();
  uint64_t size = count.size();
  fwrite(&value, sizeof(uint64_t), 1, fd);
  for (size_t i = 0; i < this->matrix.size(); ++i)
  {
    assert(size == this->matrix[i].indexes.size());
    value = this->matrix[i].table_id;
    fwrite(&value, sizeof(uint64_t), 1, fd);
  }
  
  fwrite(&size, sizeof(uint64_t), 1, fd);
  for (size_t i = 0; i < size; ++i)
  {
    for (size_t c = 0; c < this->matrix.size(); ++c)
    {
      value = this->matrix[c].indexes[i];
      fwrite(&value, sizeof(uint64_t), 1, fd);
    }
    value = this->count[i];
    fwrite(&value, sizeof(uint64_t), 1, fd);
  }
  fclose(fd);
}

void AQMatrix::load(const char * filePath, std::vector<long long>& tableIDs)
{
  this->loadHeader(filePath, tableIDs);
  if (this->nbRows > 0)
  {
    this->loadData(filePath);
  }
}

void AQMatrix::loadHeader(const char * filePath, std::vector<long long>& tableIDs)
{
  std::string answertHeader(filePath);
  answertHeader += "/AnswerHeader.a";
  FILE * fd = fopen(answertHeader.c_str(), "rb");
  if (fd == NULL)
  {
    // try old name
    std::string answertHeader(filePath);
    answertHeader += "/AnswerHeader00000.a";
    fd = fopen(answertHeader.c_str(), "rb");
    if (fd == NULL)
    {
      throw aq::generic_error(aq::generic_error::AQ_ENGINE, "cannot find aq matrix header file");
    }
  }

  uint64_t nbTable, tableId, nbGroups;
  fread(&nbTable, sizeof(uint64_t), 1, fd);
  for (uint32_t i = 0; i < nbTable; ++i)
  {
    this->matrix.push_back(column_t());
    fread(&tableId, sizeof(uint64_t), 1, fd);
    this->matrix[this->matrix.size() - 1].table_id = tableId;
    tableIDs.push_back(tableId);
  }
  
  fread(&this->totalCount, sizeof(uint64_t), 1, fd);
  fread(&this->nbRows, sizeof(uint64_t), 1, fd);
  fread(&nbGroups, sizeof(uint64_t), 1, fd);

  aq::Logger::getInstance().log(AQ_NOTICE, "aq matrix: [count:%u;rows:%u;groups:%u]\n", this->totalCount, nbRows, nbGroups);

  uint64_t countCheck = 0;
  uint64_t rowCheck = 0;
  uint64_t grpCount, grpRows;
  for (uint64_t i = 0; i < nbGroups; ++i)
  {
    fread(&grpCount, sizeof(uint64_t), 1, fd);
    fread(&grpRows, sizeof(uint64_t), 1, fd);
    
    if ((grpCount == 0) || (grpRows == 0))
      throw aq::generic_error(aq::generic_error::AQ_ENGINE, "bad value in result");

    this->groupByIndex.push_back(std::make_pair(grpCount, grpRows));
    countCheck += grpCount;
    rowCheck += grpRows;
    assert(grpCount);
    assert(grpRows);
  }
  if ((this->totalCount != countCheck) || (this->nbRows != rowCheck))
    throw aq::generic_error(aq::generic_error::AQ_ENGINE, "mismatch values in result");

  fclose(fd);
  
  this->hasCount = true;
}

void AQMatrix::loadData(const char * filePath)
{
  this->answerFormat = std::string(filePath);
  this->answerFormat += "/AnswerData%.5u.a";
  uint64_t nbPacket = (this->nbRows / aq::packet_size) + 1; 
  char * answerData = (char*)::malloc(strlen(filePath) + 18 + 1);
  size_t count_tmp = 0;
  for (uint64_t packet = 0; packet < nbPacket; ++packet)
  {
    sprintf(answerData, this->answerFormat.c_str(), packet);
    FILE * fd = fopen(answerData, "rb");
    if (fd == NULL)
    {
      free(answerData);
      throw aq::generic_error(aq::generic_error::AQ_ENGINE, "cannot find aq matrix data file %s", answerData);
    }
    uint64_t value;
    for (size_t i = 0; (i < aq::packet_size) && this->count.size() < nbRows; ++i)
    {
      for (size_t c = 0; c < this->matrix.size(); ++c)
      {
        fread(&value, sizeof(uint64_t), 1, fd);
        this->matrix[c].indexes.push_back(value);
      }
      fread(&value, sizeof(uint64_t), 1, fd);
      this->count.push_back(value);
      count_tmp += value;
    }
    fclose(fd);
  }
  free(answerData);
  if ((this->totalCount != count_tmp) || (this->nbRows != this->count.size()))
  {
    throw aq::generic_error(aq::generic_error::AQ_ENGINE, "bad matrix data file");
  }
}

void AQMatrix::prepareData(const char * filePath)
{
  this->nbRowsParsed = 0;
  this->packet = 0;
  this->nbPacket = this->nbRows / aq::packet_size; 
  this->answerFormat = std::string(filePath);
  this->answerFormat += "/AnswerData%.5u.a";
}

void AQMatrix::loadNextPacket()
{
  char * answerData = (char*)::malloc(this->answerFormat.size() + 1);
  sprintf(answerData, this->answerFormat.c_str(), this->packet);
  FILE * fd = fopen(answerData, "rb");
  if (fd == NULL)
  {
    throw aq::generic_error(aq::generic_error::AQ_ENGINE, "cannot find aq matrix data file %s", answerData);
  }
  uint64_t value;
  for (size_t i = 0; (i < aq::packet_size) && this->count.size() < nbRows; ++i)
  {
    for (size_t c = 0; c < this->matrix.size(); ++c)
    {
      fread(&value, sizeof(uint64_t), 1, fd);
      this->matrix[c].indexes.push_back(value);
    }
    fread(&value, sizeof(uint64_t), 1, fd);
    this->count.push_back(value);
  }
  fclose(fd);
  this->packet += 1;

  if (this->packet == nbPacket)
  {
    assert(this->totalCount == this->count.size());
    assert(this->nbRows == this->count.size());
  }
}

void AQMatrix::computeUniqueRow(std::vector<std::vector<size_t> >& mapToUniqueIndex, std::vector<std::vector<size_t> >& uniqueIndex) const
{
	size_t nrColumns = this->matrix.size();
	for( size_t idx = 0; idx < nrColumns; ++idx )
	{
    mapToUniqueIndex.push_back( std::vector<size_t>() );
		uniqueIndex.push_back( std::vector<size_t>() );
		if( this->matrix[idx].indexes.size() < 1 )
			continue;

		::inner_column_cmp_t cmp(this->matrix[idx].indexes);

		std::vector<size_t> index;
		index.reserve(this->matrix[idx].indexes.size());
		for (size_t idx2 = 0; idx2 < this->matrix[idx].indexes.size(); ++idx2)
			index.push_back(idx2);

		sort(index.begin(), index.end(), cmp);

		mapToUniqueIndex[idx].resize(index.size(), std::string::npos);
		uniqueIndex[idx].push_back( (size_t) this->matrix[idx].indexes[index[0]] );
		mapToUniqueIndex[idx][index[0]] = uniqueIndex[idx].size() - 1;
		for( size_t idx2 = 1; idx2 < index.size(); ++idx2 )
		{
			size_t row1 = (size_t) this->matrix[idx].indexes[index[idx2 - 1]];
			size_t row2 = (size_t) this->matrix[idx].indexes[index[idx2]];
			if( row1 != row2 )
				uniqueIndex[idx].push_back( row2 );
			mapToUniqueIndex[idx][index[idx2]] = uniqueIndex[idx].size() - 1;
		}
	}
}

void AQMatrix::compress()
{
  size_t p = 0;
  size_t index = 0;
  for (auto it = this->groupByIndex.begin(); it != this->groupByIndex.end(); ++it, ++index)
  {
    this->count[index] = 1;
    for (size_t c = 0; c < this->matrix.size(); ++c)
    {
      assert(p < this->matrix[c].indexes.size());
      this->matrix[c].indexes[index] = this->matrix[c].indexes[p];
      p += (*it).second;
    }
  }

  for (size_t c = 0; c < this->matrix.size(); ++c)
  {
    this->matrix[c].indexes.resize(index);
  }
  this->count.resize(index);
}

void AQMatrix::writeTemporaryTable()
{
  uint64_t packet = 0;
  char filename[1024];
  FILE * fd;
  std::vector<std::map<uint64_t, FILE*> > fds(this->matrix.size());
  uint32_t status = 11;
  uint32_t invalid = 0;
  uint32_t pos = 0;
  uint64_t grpIndex = 1;
  uint64_t size = (*this->matrix.begin()).indexes.size();
  for (uint64_t i = 0; i < size; ++i, ++grpIndex)
  {
    for (size_t c = 0; c < this->matrix.size(); ++c)
    {
      pos = static_cast<uint32_t>(this->matrix[c].indexes[i]);
      packet = pos / settings.packSize;
      pos = (pos - 1) % settings.packSize;
      auto it = fds[c].find(packet);
      if (it == fds[c].end())
      {
        sprintf(filename, "%s/B001REG%.4uTMP%.4uP%.12u.TMP", this->settings.szTempPath1, this->matrix[c].table_id, this->uid, packet);
        fd = fopen(filename, "wb");
        if (fd == NULL)
        {
          throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "cannot create temporary table file '%s'", filename);
        }
        if (!fds[c].insert(std::make_pair(packet, fd)).second)
        {
          throw aq::generic_error(aq::generic_error::GENERIC, "MEMORY ERROR");
        }
        fwrite(&status, sizeof(uint32_t), 1, fd);
      }
      else
      {
        fd = it->second;
      }
      
      fwrite(&invalid, sizeof(uint32_t), 1, fd);
      fwrite(&pos, sizeof(uint32_t), 1, fd);
    }
  }
  
  for (size_t c = 0; c < this->matrix.size(); ++c)
  {
    for (auto& v : fds[c])
    {
      fclose(v.second);
    }
  }
  
  // FIXME : generate empty file => this is temporary
  char tableName[128];
  for (auto it = this->matrix.begin(); it != this->matrix.end(); ++it)
  {
    Table::Ptr table = this->baseDesc.getTable((*it).table_id);
    uint64_t n = table->TotalCount / this->settings.packSize;
    for (uint64_t i = 0; i <= n; ++i)
    {
      sprintf(filename, "%s/B001REG%.4uTMP%.4uP%.12u.TMP", this->settings.szTempPath1, (*it).table_id, this->uid, i);
      FILE * fd = fopen(filename, "ab");
      fclose(fd);
    }
    sprintf(tableName, "B001REG%.4uTMP%.4uP%.12u", (*it).table_id, this->uid, n + 1);
    (*it).tableName = tableName;
    (*it).baseTableName = table->getName();
  }
}

void AQMatrix::dump(std::ostream& os) const
{
  size_t size = (*this->matrix.begin()).indexes.size();
  
  for (size_t c = 0; c < this->matrix.size(); ++c)
  {
    os << this->matrix[c].table_id << " ";
  }
  os << std::endl;

  for (size_t i = 0; i < size; ++i)
  {
    for (size_t c = 0; c < this->matrix.size(); ++c)
    {
      os << this->matrix[c].indexes[i] << " ";
    }
    os << std::endl;
  }
}
