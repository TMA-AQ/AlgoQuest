#include "AQMatrix.h"
#include <aq/Utilities.h>
#include <aq/Exceptions.h>
#include "Table.h"
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
	struct column_cmp_t
	{
	public:
		column_cmp_t(const std::vector<uint64_t>& lessThanColumn)
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
}

AQMatrix::AQMatrix()
	: totalCount(0),
		nbRows(0),
		hasCount(false)
{
}

AQMatrix::AQMatrix(const AQMatrix& source)
	: totalCount(source.totalCount),
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
	this->indexes.resize(nbTables, std::vector<uint64_t>(rows, 0));
	for (size_t i = 0; i < rows; ++i)
	{
		std::for_each(this->indexes.begin(), this->indexes.end() - 1 , [&] (std::vector<uint64_t>& v) {
			v.push_back(i);
		});
		(*this->indexes.rbegin()).push_back(1);
	}
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

	this->indexes.clear();

	pszBigBuffer = new char[STR_BIG_BUF_SIZE];
	boost::scoped_array<char> pszBigBufferDel( pszBigBuffer );
	llong lineNr = 0;
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
				for( size_t idx = 0; idx < fields.size(); ++idx )
				{
					this->indexes.push_back(std::vector<uint64_t>());

					// add or check 'Count'
					if( this->hasCount && (idx + 1 == fields.size()) )
					{
						assert(lastField == "COUNT");
					}
					else // add or check id table
					{
						char* tableNr = strchr(fields[idx], ':');
						std::string tableNrStr(tableNr + 1);
						Trim(tableNrStr);
						llong tableID = boost::lexical_cast<llong>(tableNrStr.c_str());
						tableIDs.push_back(tableID);
					}
				}
				continue;
			}
		case 1: //second line is count(*)
		{
			std::string pszTrim(psz);
			Trim(pszTrim);
			this->totalCount = boost::lexical_cast<unsigned>(pszTrim.c_str());
			continue;
		}
		case 2: //third line is the number of rows in the answer
		{
			std::string pszTrim(psz);
			Trim(pszTrim);
			this->nbRows = boost::lexical_cast<unsigned>(pszTrim.c_str());
			continue;
		}
		default:;
		}

		//
		// Lines must start with a field separator
		if( psz[0] != fieldSeparator )
			continue;
		if( fields.size() != indexes.size() )
			throw generic_error(generic_error::INVALID_TABLE, "");
		assert( fields.size() == this->indexes.size() );
		std::string pszTrim;
		for( size_t idx = 0; idx < fields.size(); ++idx )
		{
			pszTrim = fields[idx];
			Trim(pszTrim);
			nVal = boost::lexical_cast<unsigned>(pszTrim.c_str());
			this->indexes[idx].push_back(nVal);
		}
	}
}

void AQMatrix::computeUniqueRow(std::vector<std::vector<size_t> >& mapToUniqueIndex, std::vector<std::vector<size_t> >& uniqueIndex) const
{
	//each column in the table holds a list of row indexes
	//compute a column containing unique and sorted row indexes
	//also compute a mapping between the original row indexes and the
	//sorted and unique indexes
	size_t nrColumns = this->indexes.size();
	if( this->hasCount )
		--nrColumns;
	for( size_t idx = 0; idx < nrColumns; ++idx )
	{
		mapToUniqueIndex.push_back( std::vector<size_t>() );
		uniqueIndex.push_back( std::vector<size_t>() );
		if( this->indexes[idx].size() < 1 )
			continue;

		column_cmp_t cmp(this->indexes[idx]);

		std::vector<size_t> index;
		index.reserve(this->indexes[idx].size());
		for (size_t idx2 = 0; idx2 < this->indexes[idx].size(); ++idx2)
			index.push_back(idx2);

		sort(index.begin(), index.end(), cmp);

		mapToUniqueIndex[idx].resize(index.size(), -1);
		uniqueIndex[idx].push_back( (size_t) this->indexes[idx][index[0]] );
		mapToUniqueIndex[idx][index[0]] = uniqueIndex[idx].size() - 1;
		for( size_t idx2 = 1; idx2 < index.size(); ++idx2 )
		{
			size_t row1 = (size_t) this->indexes[idx][index[idx2 - 1]];
			size_t row2 = (size_t) this->indexes[idx][index[idx2]];
			if( row1 != row2 )
				uniqueIndex[idx].push_back( row2 );
			mapToUniqueIndex[idx][index[idx2]] = uniqueIndex[idx].size() - 1;
		}
	}
}

class ColumnMapper
{
public:
		void getValue(size_t index, ColumnItem::Ptr value)
		{
			// TODO
		}
};

void AQMatrix::groupBy(const std::map<size_t, std::vector<size_t> >& columnsByTableId)
{
	if (this->indexes.size() == 0)
		return;

	size_t nbColumns = 0;

	for (std::map<size_t, std::vector<size_t> >::const_iterator it_t = columnsByTableId.begin(); it_t != columnsByTableId.end(); ++it_t)
	{
		std::cout << "TABLE: " << it_t->first << std::endl;
		std::cout << "  COLUMNS: ";
		for (std::vector<size_t>::const_iterator it_c = it_t->second.begin(); it_c != it_t->second.end(); ++it_c)
		{
			++ nbColumns;
			std::cout << *it_c << " ";
		}
		std::cout << std::endl;
	}

	if (this->indexes.size() < 0)
	{
		return;
	}

	//
	// TODO : check index length integrity

	size_t size = this->indexes[0].size();
	groupByIndex.resize(size, 0);
	std::vector<std::pair<size_t, std::vector<ColumnItem::Ptr> > > items(10, std::make_pair(0, std::vector<ColumnItem::Ptr>(nbColumns))); // FIXME : the size must be a settings
	std::vector<ColumnMapper> columnsMapper(nbColumns);
	size_t lastItem = 0;

	std::set<size_t> treatedRows;
	std::set<size_t> indexToTreat;
	for (size_t i = 0; i < size; ++i) indexToTreat.insert(i);

	while ((treatedRows.size() < indexes.size()) && (indexToTreat.size() != 0))
	{

		for (std::set<size_t>::iterator it = indexToTreat.begin(); it != indexToTreat.end(); ++it)
		{
			size_t index = *it;

			// Check if this row has been treated
			assert(treatedRows.find(index) == treatedRows.end());

			// Load Row
			std::pair<size_t, std::vector<ColumnItem::Ptr> >& row = items[lastItem];
			row.first = index;
			for (size_t i = 0; i < columnsMapper.size(); ++i)
			{
				columnsMapper[i].getValue(index, row.second[i]);
			}

			// check if this row is identical to a preceding row stored in items
			if (lastItem > 0)
			{
				size_t i = 0;
				for (i = 0; i < lastItem; ++i)
				{
					//if (std::equal(row.second.begin(), row.second.end(), items[i].second.begin()))
					//{
					//	break;
					//}

					// FIXME
					if (index % 1000)
					{
						break;
					}

				}

				if (i == lastItem)
				{
					// No identical row found
					if (lastItem == items.size() - 1)
					{
						// items is not big enough to contains this row => do not keep it
					}
					else
					{
						// items can contain this row => mark it as treated and give a new group-by index
						lastItem ++;
						treatedRows.insert(index);
						assert(row.first == index);
						groupByIndex[index] = index;
					}
				}
				else
				{
					// an identical row has been found => mark as treated and give its group-by index
					treatedRows.insert(index);
					groupByIndex[index] = items[i].first;
				}

			}
			else
			{
				// this is the first row treated a new group-by index
				lastItem ++;
				treatedRows.insert(index);
				assert(row.first == index);
				groupByIndex[index] = index;
			}

		}

		// all row in items are treated and group by is complete for this rows (it is possible to apply a callback on it)
		items.clear();
		lastItem = 0;

		// remove treated row
		std::for_each(treatedRows.begin(), treatedRows.end(), [&] (size_t v) {
			indexToTreat.erase(v);
		});

		assert((treatedRows.size() + indexToTreat.size()) == size);

	}

	// FIXME : for test
	size_t gbid = 0;
	for (size_t i = 0; i < groupByIndex.size(); ++i)
	{
		if (groupByIndex[i] > gbid)
		{
			gbid = groupByIndex[i];
		}
		groupByIndex[i] = gbid;
	}

}