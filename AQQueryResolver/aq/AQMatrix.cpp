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
		inner_column_cmp_t(const std::vector<uint64_t>& lessThanColumn)
			: m_lessThanColumn(lessThanColumn)
		{
		}
		bool operator()(size_t idx1, size_t idx2)
		{
			return m_lessThanColumn[idx1] < m_lessThanColumn[idx2];
		}
	private:
		const std::vector<uint64_t>& m_lessThanColumn;
	};

	struct row_cmp_t
	{
	public:
		row_cmp_t(std::vector<ColumnMapper::Ptr>& _columnMapper)
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
			std::for_each(columnMapper.begin(), columnMapper.end(), [&] (ColumnMapper::Ptr& c) {
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
		std::vector<ColumnMapper::Ptr>& columnMapper;
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

void AQMatrix::simulate(size_t rows, size_t nbTables)
{
	//this->hasCount = true;
	//this->totalCount = rows;
	//this->nbRows = rows;
	//this->matrix.resize(nbTables);
	//for (size_t i = 0; i < rows; ++i)
	//{
	//	std::for_each(this->matrix.begin(), this->matrix.end() - 1 , [&] (column_t& c) {
	//		c.indexes.push_back(i);
	//		c.grpKey.push_back(i % (rows / 100));
	//	});
	//}
	//this->count.resize(rows, 1);
}

void AQMatrix::clear()
{
  matrix.clear();
	count.clear();
  groupByIndex.clear();
	totalCount = 0;
	nbRows = 0;
  size = 0;
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
  std::string answertHeader(filePath);
  answertHeader += "/AnswerHeader00000.a";
  FILE * fd = fopen(answertHeader.c_str(), "rb");
  if (fd == NULL)
  {
    throw aq::generic_error(aq::generic_error::AQ_ENGINE, "cannot find aq matrix header file");
  }

  uint64_t nbTable, tableId, count, nbRows, nbGroups;
  fread(&nbTable, sizeof(uint64_t), 1, fd);
  for (uint32_t i = 0; i < nbTable; ++i)
  {
    this->matrix.push_back(column_t());
    fread(&tableId, sizeof(uint64_t), 1, fd);
    this->matrix[this->matrix.size() - 1].table_id = tableId;
    tableIDs.push_back(tableId);
  }
  
  fread(&count, sizeof(uint64_t), 1, fd);
  fread(&nbRows, sizeof(uint64_t), 1, fd);
  fread(&nbGroups, sizeof(uint64_t), 1, fd);

  uint64_t countCheck = 0;
  uint64_t rowCheck = 0;
  uint64_t grpCount, grpRows;
  for (uint64_t i = 0; i < nbGroups; ++i)
  {
    fread(&grpCount, sizeof(uint64_t), 1, fd);
    fread(&grpRows, sizeof(uint64_t), 1, fd);
    
    // FIXME
    if (grpCount == 0)
      grpCount = count - countCheck;
    if (grpRows == 0)
      grpRows = nbRows - rowCheck;

    if ((grpCount == 0) || (grpRows == 0))
      throw aq::generic_error(aq::generic_error::AQ_ENGINE, "bad value in result");

    this->groupByIndex.push_back(std::make_pair(grpCount, grpRows));
    countCheck += grpCount;
    rowCheck += grpRows;
    assert(grpCount);
    assert(grpRows);
  }
  if ((count != countCheck) || (nbRows != rowCheck))
    throw aq::generic_error(aq::generic_error::AQ_ENGINE, "mismatch values in result");

  fclose(fd);
  
  std::string answerData(filePath);
  answerData += "/AnswerData00000.a";
  fd = fopen(answerData.c_str(), "rb");
  if (fd == NULL)
  {
    throw aq::generic_error(aq::generic_error::AQ_ENGINE, "cannot find aq matrix data file");
  }
  uint64_t value;
  for (uint64_t i = 0; i < nbGroups; ++i)
  {
    for (uint64_t j = 0; j < this->groupByIndex[i].second; ++j)
    {
      for (size_t c = 0; c < this->matrix.size(); ++c)
      {
        fread(&value, sizeof(uint64_t), 1, fd);
        this->matrix[c].indexes.push_back(value);
      }
      fread(&value, sizeof(uint64_t), 1, fd);
      this->count.push_back(value);
    }
  }

  if (fd != NULL)
    fclose(fd);
  this->hasCount = true;
  this->totalCount = this->count.size();
	this->nbRows = this->count.size();
  this->size = this->count.size();

  aq::Logger::getInstance().log(AQ_INFO, "%u rows to proceed (count:%u;group:%u)\n", this->nbRows, count, nbGroups);
}

void AQMatrix::load(const char * filePath, const char fieldSeparator, std::vector<long long>& tableIDs)
{
	char		*pszAnswer = NULL;
	char		*psz = NULL;
	FILE		*pFIn = NULL;
	uint64_t nVal = 0;
	double	 dVal = 0;
	char		*pszBigBuffer = NULL;
	bool		 foundFirstLine = false;

	pFIn = fopenUTF8( filePath, "rt" );
	FileCloser fileCloser( pFIn );
	if( !pFIn )
		throw generic_error(generic_error::COULD_NOT_OPEN_FILE, "");

	this->matrix.clear();
  this->count.clear();
  this->groupByIndex.clear();

	pszBigBuffer = new char[STR_BIG_BUF_SIZE];
	boost::scoped_array<char> pszBigBufferDel( pszBigBuffer );
	llong lineNr = 0;
	size_t expected_size = 0;
	std::vector<std::pair<size_t, size_t> > description;

  // read until the first ';'
  char c = '\0';
  while (!feof(pFIn) && c != ';')
  {
    fread(&c, sizeof(char), 1, pFIn);
  }
  if (c == ';')
  {
    fseek(pFIn, -1, SEEK_CUR);
  }

	while( psz = ReadValidLine( pFIn, pszBigBuffer, STR_BIG_BUF_SIZE, 0 ) )
	{
		std::vector<char*> fields;
		splitLine( psz, fieldSeparator, fields, true );
		switch( lineNr++ )
		{
		case 0: //first line is a description
			{
				if( psz[0] != fieldSeparator )
				{
          throw aq::generic_error(aq::generic_error::AQ_ENGINE, "bad format of aq engine matrix");
				}
				//answer is empty, but this is not an error
				if( fields.size() < 1 )
					return;

				//check for "Count"
				std::string lastField = fields[fields.size() - 1];
				strtoupr(lastField);
				Trim(lastField);
				this->hasCount = (lastField == "COUNT");

				//initialize columns
				expected_size = fields.size();
				for( size_t idx = 0; idx < fields.size(); ++idx )
				{
					column_t c;

					if( this->hasCount && (idx + 1 == fields.size()) )
					{
						description.push_back(std::make_pair(4, 0));
						assert(lastField == "COUNT");
					}
					else 
					{
						std::string field = fields[idx];
						char* tableNr = strchr(fields[idx], ':');
						std::string tableNrStr(tableNr + 1);
						Trim(tableNrStr);
						llong tableID = boost::lexical_cast<llong>(tableNrStr.c_str());
						if (field.find("Num_table") != std::string::npos)
						{
							tableIDs.push_back(tableID);
							c.table_id = tableID;
							description.push_back(std::make_pair(1, tableID));
						}
						else if (field.find("Group_Key") != std::string::npos)
						{
							c.table_id = tableID;
							description.push_back(std::make_pair(2, tableID));
						}
						else if (field.find("Order_Key") != std::string::npos)
						{
							c.table_id = tableID;
							description.push_back(std::make_pair(3, tableID));
						}
						else
						{
							throw generic_error(generic_error::INVALID_FILE, "bad format of aq engine matrix");
						}
					}

          if (std::find_if(this->matrix.begin(), this->matrix.end(), boost::bind(std::equal_to<size_t>(), c.table_id, boost::bind(&column_t::table_id, _1))) == this->matrix.end())
            this->matrix.push_back(c);
				}
				continue;
			}
		case 1: //second line is count(*)
		{
			std::string pszTrim(psz);
			Trim(pszTrim);
			this->totalCount = boost::lexical_cast<uint64_t>(pszTrim.c_str());
			continue;
		}
		case 2: //third line is the number of rows in the answer
		{
			std::string pszTrim(psz);
			Trim(pszTrim);
			this->nbRows = boost::lexical_cast<uint64_t>(pszTrim.c_str());
			continue;
		}
		default:;
		}

		//
		// Lines must start with a field separator
		if( psz[0] != fieldSeparator )
			continue;
		if( fields.size() != expected_size )
			throw generic_error(generic_error::INVALID_TABLE, "");
		assert( fields.size() == expected_size );
		std::string pszTrim;
		for( size_t idx = 0; idx < fields.size(); ++idx )
		{
			pszTrim = fields[idx];
			Trim(pszTrim);
			nVal = boost::lexical_cast<uint64_t>(pszTrim.c_str());
			switch (description[idx].first)
			{
			case 1: this->matrix[idx].indexes.push_back(nVal); break;
			case 2: this->matrix[idx].grpKey.push_back(nVal); break;
			case 3: this->matrix[idx].orderKey.push_back(nVal); break;
			case 4: this->count.push_back(nVal); break;
			default: assert(false);
			}
		}
	}

  this->size = 0;
	for (size_t i = 0; i < this->matrix.size(); ++i)
	{
		if (this->size == 0)
			this->size = this->matrix[i].indexes.size();
		else if (this->size != this->matrix[i].indexes.size())
		{
			throw generic_error(generic_error::INVALID_TABLE, "columns answer are not equals");
		}
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

		inner_column_cmp_t cmp(this->matrix[idx].indexes);

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

// void AQMatrix::groupBy(const std::map<size_t, std::vector<std::pair<size_t, aq::ColumnType> > >& columnsByTableId)
void AQMatrix::groupBy(std::vector<aq::ColumnMapper::Ptr>& columnsMapper)
{
	if (this->matrix.size() == 0)
		return;

	size_t size = this->matrix[0].indexes.size();

	//
	// TODO : check index length integrity
	//for (size_t index = 0; index < size; ++index)
	//{
	//	// Load Row and compute group by key
	//	size_t r = 0;
	//	group_by_key_t gk;
	//	gk.len = 0;
	//	gk.value = NULL;
	//	ColumnItem::Ptr c;
	//	for (size_t i = 0; i < columnsMapper.size(); ++i)
	//	{
	//		c = columnsMapper[i].loadValue(index);
	//		size_t pos = gk.len;
	//		gk.len += sizeof(c->numval);
	//		if (gk.value == NULL)
	//			gk.value = static_cast<uint8_t*>(::malloc(gk.len * sizeof(uint8_t)));
	//		else
	//			gk.value = static_cast<uint8_t*>(::realloc(gk.value, gk.len * sizeof(uint8_t)));
	//		::memcpy(gk.value + pos, &c->numval, sizeof(c->numval));
	//	}
	//	this->groupByIndex[gk].push_back(index);
	//}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	
	// ColumnItem::Ptr item;
	row_cmp_t row_cmp(columnsMapper);
	std::set<index_holder_t, index_holder_cmp_t> indexes_to_sort;
	for (size_t i = 0; i < size; ++i)
	{
		if ((i % 100000) == 0) aq::Logger::getInstance().log(AQ_DEBUG, "%u index sorted\n", i);
		index_holder_t ih = { i, row_cmp };
		indexes_to_sort.insert(ih);
	}

	group_by_key_t gk = { 0, NULL } ;
	//for (std::set<index_holder_t, index_holder_cmp_t>::const_iterator it = indexes_to_sort.begin(); it != indexes_to_sort.end(); ++it)
	//	this->groupByIndex[gk].push_back((*it).index);
		
	aq::Logger::getInstance().log(AQ_DEBUG, "%u group\n", groupByIndex.size());
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
  uint64_t invalid = 0;
  uint64_t pos = 0;
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
      
      fwrite(&grpIndex, sizeof(uint64_t), 1, fd);
      fwrite(&invalid, sizeof(uint64_t), 1, fd);
      fwrite(&pos, sizeof(uint64_t), 1, fd);
    }
  }
  
  for (size_t c = 0; c < this->matrix.size(); ++c)
  {
    std::for_each(fds[c].begin(), fds[c].end(), [] (std::pair<uint64_t, FILE*> v) { fclose(v.second); });
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