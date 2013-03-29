#include "AQMatrix.h"
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
				row1[pos] = c->loadValue(idx1);
				row2[pos] = c->loadValue(idx2);
				++pos;
			});
			// and compare
			for (pos = 0; pos < row1.size(); ++pos)
			{
				if (lessThan(*row1[pos], *row2[pos])) return true;
				else if (lessThan(*row2[pos], *row1[pos])) return false;
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

AQMatrix::AQMatrix(const TProjectSettings& _settings)
	: settings(_settings),
		totalCount(0),
		nbRows(0),
		hasCount(false)
{
}

AQMatrix::AQMatrix(const AQMatrix& source)
	: settings(source.settings),
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
	this->hasCount = true;
	this->totalCount = rows;
	this->nbRows = rows;
	this->matrix.resize(nbTables);
	for (size_t i = 0; i < rows; ++i)
	{
		std::for_each(this->matrix.begin(), this->matrix.end() - 1 , [&] (column_t& c) {
			c.indexes.push_back(i);
			c.grpId.push_back(i % (rows / 100));
		});
	}
	this->count.resize(rows, 1);
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

	pszBigBuffer = new char[STR_BIG_BUF_SIZE];
	boost::scoped_array<char> pszBigBufferDel( pszBigBuffer );
	llong lineNr = 0;
	size_t expected_size = 0;
	std::vector<std::pair<size_t, size_t> > description;
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
					return;
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
						else if (field.find("Group") != std::string::npos)
						{
							description.push_back(std::make_pair(2, tableID));
						}
						else if (field.find("Order") != std::string::npos)
						{
							description.push_back(std::make_pair(3, tableID));
						}
						else
						{
							throw generic_error(generic_error::INVALID_FILE, "bad format of aq engine matrix");
						}
					}

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
			case 2: this->matrix[idx].indexes.push_back(nVal); break;
			case 3: this->matrix[idx].indexes.push_back(nVal); break;
			case 4: this->count.push_back(nVal); break;
			default: assert(false);
			}
		}
	}
}

void AQMatrix::computeUniqueRow(std::vector<std::vector<size_t> >& mapToUniqueIndex, std::vector<std::vector<size_t> >& uniqueIndex) const
{
	//each column in the table holds a list of row indexes
	//compute a column containing unique and sorted row indexes
	//also compute a mapping between the original row indexes and the
	//sorted and unique indexes
	size_t nrColumns = this->matrix.size();
	if( this->hasCount )
		--nrColumns;
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
	for (std::set<index_holder_t, index_holder_cmp_t>::const_iterator it = indexes_to_sort.begin(); it != indexes_to_sort.end(); ++it)
		this->groupByIndex[gk].push_back((*it).index);
		
	aq::Logger::getInstance().log(AQ_DEBUG, "%u group\n", groupByIndex.size());
}