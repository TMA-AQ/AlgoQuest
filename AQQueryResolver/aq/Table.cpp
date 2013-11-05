#include "Base.h"
#include "Table.h"
#include "RowProcess_Intf.h"

#include <aq/Utilities.h>
#include <aq/Exceptions.h>
#include <aq/DateConversion.h>
#include <aq/FileMapper.h>
#include <aq/Timer.h>
#include <aq/Logger.h>

#include <cassert>
#include <memory>
#include <algorithm>
#include <set>
#include <boost/scoped_array.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

//------------------------------------------------------------------------------
#define STR_BIG_BUF_SIZE 1048576

namespace aq
{

//------------------------------------------------------------------------------
Table::Table()
  : 
	ID(0), 
	HasCount(false), 
	TotalCount(0), 
	GroupByApplied(false),
	OrderByApplied(false),
	NoAnswer(false)
{
	memset(szBuffer, 0, STR_BUF_SIZE);
}

//------------------------------------------------------------------------------
Table::Table(const std::string& name, unsigned int ID, bool _temporary)
  : 
	ID(ID), 
	HasCount(false), 
	TotalCount(0), 
	GroupByApplied(false), 
	OrderByApplied(false),
	NoAnswer(false),
  temporary(_temporary)
{
	this->setName( name );
	memset(szBuffer, 0, STR_BUF_SIZE);
}

//------------------------------------------------------------------------------
Table::Table(const Table& source)
  : 
	ID(source.ID), 
	HasCount(source.HasCount), 
	TotalCount(source.TotalCount), 
	GroupByApplied(source.GroupByApplied), 
	OrderByApplied(source.OrderByApplied),
	NoAnswer(source.NoAnswer),
  temporary(source.temporary)
{
  this->setName(source.Name);
	memset(szBuffer, 0, STR_BUF_SIZE);
  for (auto& c : source.Columns)
  {
    this->Columns.push_back(new Column(*c));
  }
}

//------------------------------------------------------------------------------
Table::~Table()
{
}

//------------------------------------------------------------------------------
Table& Table::operator=(const Table& source)
{
  if (this != &source)
  {
    ID = source.ID; 
    HasCount = source.HasCount;
    TotalCount = source.TotalCount;
    GroupByApplied = source.GroupByApplied;
    OrderByApplied = source.OrderByApplied;
    NoAnswer = source.NoAnswer;
    temporary = source.temporary;
    this->setName(source.Name);
    memset(szBuffer, 0, STR_BUF_SIZE);
    for (auto& c : source.Columns)
    {
      this->Columns.push_back(new Column(*c));
    }
  }
  return *this;
}

//------------------------------------------------------------------------------
Column::Ptr Table::getColumn(const std::string& columnName) const
{
  std::string aux = columnName;
  boost::trim(aux);
  boost::to_upper(aux);
  for (auto& c : this->Columns)
  {
    if (c->getName() == aux)
    {
      return c;
    }
  }
  throw aq::generic_error(aq::generic_error::INVALID_QUERY, "cannot find table [%s]", columnName.c_str());
}

//------------------------------------------------------------------------------
int Table::getColumnIdx( const std::string& name ) const
{
	std::string auxName = name;
	boost::to_upper(auxName);
	boost::trim(auxName);
	for( size_t idx = 0; idx < this->Columns.size(); ++idx )
		if( auxName == this->Columns[idx]->getName() )
			return static_cast<int>(idx);
	return -1;
}

//------------------------------------------------------------------------------
void Table::computeUniqueRow(Table& aqMatrix, std::vector<std::vector<size_t> >& mapToUniqueIndex, std::vector<std::vector<size_t> >& uniqueIndex) const
{
	//each column in the table holds a list of row indexes
	//compute a column containing unique and sorted row indexes
	//also compute a mapping between the original row indexes and the
	//sorted and unique indexes
	size_t nrColumns = aqMatrix.Columns.size();
	if( aqMatrix.HasCount )
		--nrColumns;
	for( size_t idx = 0; idx < nrColumns; ++idx )
	{
		mapToUniqueIndex.push_back( std::vector<size_t>() );
		uniqueIndex.push_back( std::vector<size_t>() );
		if( aqMatrix.Columns[idx]->Items.size() < 1 )
			continue;
		Column::inner_column_cmp_t cmp(*(aqMatrix.Columns[idx].get()));
		std::vector<size_t> index;
		for( size_t idx2 = 0; idx2 < aqMatrix.Columns[idx]->Items.size(); ++idx2 )
			index.push_back( idx2 );
		std::sort( index.begin(), index.end(), cmp );

		mapToUniqueIndex[idx].resize( index.size(), -1 );
		uniqueIndex[idx].push_back( (size_t) aqMatrix.Columns[idx]->Items[index[0]]->numval );
		mapToUniqueIndex[idx][index[0]] = uniqueIndex[idx].size() - 1;
		for( size_t idx2 = 1; idx2 < index.size(); ++idx2 )
		{
			size_t row1 = (size_t) aqMatrix.Columns[idx]->Items[index[idx2 - 1]]->numval;
			size_t row2 = (size_t) aqMatrix.Columns[idx]->Items[index[idx2]]->numval;
			if( row1 != row2 )
				uniqueIndex[idx].push_back( row2 );
			mapToUniqueIndex[idx][index[idx2]] = uniqueIndex[idx].size() - 1;
		}
	}
}

//------------------------------------------------------------------------------
void Table::load(const char * path, uint64_t packetSize)
{
  if (this->temporary)
  {
    for (auto& c : this->Columns) 
    {
      aq::TemporaryColumnMapper::Ptr cm(new aq::TemporaryColumnMapper(path, this->ID, c->ID, c->Type, c->Size, packetSize));
      c->loadFromTmp(c->Type, cm);
    }
  }
  else
  {
    throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "table load not implemented");
  }
}

//------------------------------------------------------------------------------
void Table::loadColumn(Column::Ptr col, const std::vector<size_t>& uniqueIndex, const std::vector<size_t>& mapToUniqueIndex, const Column::Ptr columnType, const Settings& pSettings, const Base& BaseDesc)
{		
	std::string tableName = columnType->getTableName();
	size_t tableID = BaseDesc.getTable(tableName)->ID;

	//load the required column values
	llong currentNumPack = -1;

	unsigned char		*pTmpBuf = NULL;
	size_t           nTmpBufSize = 0;
	size_t           nBinItemSize = 0;

	pTmpBuf			  = NULL;
	nBinItemSize	= 0;
	switch( columnType->Type )
	{
	case COL_TYPE_INT:
		nTmpBufSize		= 1000;
		nBinItemSize	= 4;
		break;
	case COL_TYPE_BIG_INT:
	case COL_TYPE_DATE:
	case COL_TYPE_DOUBLE:
		nTmpBufSize		= 1000;
		nBinItemSize	= 8;
		break;
	case COL_TYPE_VARCHAR:
		nTmpBufSize		= columnType->Size + 1000;	// reserve 1000 bytes extra space for text mode !
		nBinItemSize	= columnType->Size;
		break;
	}

	pTmpBuf = new unsigned char[nTmpBufSize];
	pTmpBuf[nBinItemSize] = 0;
	boost::scoped_array<unsigned char> pTmpBufDel( pTmpBuf ); //!!!debug13
	
	boost::shared_ptr<aq::FileMapper> prmMapper;
	boost::shared_ptr<aq::FileMapper> thesaurusMapper;

	aq::Timer realValuesTimer;
	Column::Ptr columnValues = new Column( *columnType );
	for( size_t idx2 = 0; idx2 < uniqueIndex.size(); ++idx2 )
	{
		llong absRaw = uniqueIndex[idx2];
		size_t numPack = (size_t) (absRaw / pSettings.packSize);
		llong packOffset = absRaw % pSettings.packSize;

		// aq::Logger::getInstance().log(AQ_DEBUG, "process row %u => unique index %ld on packet %u offset %ld\n", idx2, absRaw, numPack, packOffset);

		if( numPack != currentNumPack )
		{
			std::string dataPath = pSettings.dataPath;
			sprintf( szBuffer, "B001T%.4uC%.4uV01P%.12u", tableID, columnType->ID, numPack ); // FIXME : use a function
			dataPath += szBuffer;

			std::string prmFilePath = dataPath + ".prm";
			std::string theFilePath = dataPath + ".the";

			 aq::Logger::getInstance().log(AQ_DEBUG, "Open prm file %s\n", prmFilePath.c_str());
			 aq::Logger::getInstance().log(AQ_DEBUG, "Open thesaurus %s\n", theFilePath.c_str());

			prmMapper.reset(new aq::FileMapper(prmFilePath.c_str()));
			thesaurusMapper.reset(new aq::FileMapper(theFilePath.c_str()));

			currentNumPack = numPack;
		}
		
		int prmFileItemSize = 4;
		int prmOffset = ((long) packOffset * prmFileItemSize);
		int theOffset;
		prmMapper->read(&theOffset, prmOffset, prmFileItemSize);

		switch( columnType->Type )
		{
		case COL_TYPE_INT:
			{
				int *pnItemData;
				pnItemData = (int*)( pTmpBuf );
				thesaurusMapper->read(pTmpBuf, nBinItemSize * theOffset, nBinItemSize);
				if( *pnItemData == 'NULL' )
					columnValues->Items.push_back( NULL );
				else
					columnValues->Items.push_back( new ColumnItem( *pnItemData ) );
			}
			break;
		case COL_TYPE_BIG_INT:
		case COL_TYPE_DATE:
			{
				long long *pnItemData;
				pnItemData = (long long*)( pTmpBuf );
				thesaurusMapper->read(pTmpBuf, nBinItemSize * theOffset, nBinItemSize);
				if( *pnItemData == 'NULL' )
					columnValues->Items.push_back( NULL );
				else
					columnValues->Items.push_back( new ColumnItem( (double) *pnItemData ) );
			}
			break;
		case COL_TYPE_DOUBLE:
			{
				double *pdItemData;
				pdItemData = (double*)( pTmpBuf );
				thesaurusMapper->read(pTmpBuf, nBinItemSize * theOffset, nBinItemSize);
				if( *pdItemData == 'NULL' )
					columnValues->Items.push_back( NULL );
				else
					columnValues->Items.push_back( new ColumnItem( *pdItemData ) );
			}
			break;
		case COL_TYPE_VARCHAR:
			{	
				thesaurusMapper->read(pTmpBuf, nBinItemSize * theOffset, nBinItemSize);
				if( strcmp((char*)pTmpBuf, "NULL") == 0 )
					columnValues->Items.push_back( NULL );
				else
					columnValues->Items.push_back( new ColumnItem( std::string((char*)pTmpBuf) ) );
			}
			break;
		}
	}
	aq::Logger::getInstance().log(AQ_DEBUG, "Turn %u numerics references of column %s into real values: Time Elapsed = %s\n", 
		uniqueIndex.size(),
		columnType->getName().c_str(), aq::Timer::getString(realValuesTimer.getTimeElapsed()).c_str());

	for( size_t idx2 = 0; idx2 < mapToUniqueIndex.size(); ++idx2 ) {
		assert(mapToUniqueIndex[idx2] < columnValues->Items.size());
		col->Items.push_back( columnValues->Items[mapToUniqueIndex[idx2]] );
	}
}

//------------------------------------------------------------------------------
std::vector<Column::Ptr> Table::getColumnsByName( std::vector<Column::Ptr>& columns ) const
{
	std::vector<Column::Ptr> correspondingColumns;
	for( size_t idx = 0; idx < columns.size(); ++idx )
	{
		bool found = false;
		for( size_t idx2 = 0; idx2 < this->Columns.size(); ++idx2 )
			if( columns[idx]->getName() == this->Columns[idx2]->getName() )
			{
				correspondingColumns.push_back( this->Columns[idx2] );
				found = true;
				break;
			}
		if( !found )
		{
			//we have an intermediary column, not a table column
			//check that it's valid
			if( this->Columns.size() > 0 )
				if( columns[idx]->Items.size() != this->Columns[0]->Items.size() )
					throw generic_error(generic_error::INVALID_QUERY, "");
			correspondingColumns.push_back( columns[idx] );
		}
	}
	return correspondingColumns;
}

//------------------------------------------------------------------------------
void Table::setName( const std::string& name )
{
	this->OriginalName = name;
	this->Name = name;
	boost::to_upper(this->Name);
	boost::trim(this->Name);
}

//------------------------------------------------------------------------------
const std::string& Table::getName() const
{
	return this->Name;
}

//------------------------------------------------------------------------------
const std::string& Table::getOriginalName() const
{
	return this->OriginalName;
}

//------------------------------------------------------------------------------
void Table::dumpRaw( std::ostream& os )
{
  if( this->Columns.size() == 0 )
    throw generic_error(generic_error::INVALID_TABLE, "");
  size_t nrColumns = this->Columns.size();
  if( this->HasCount )
    --nrColumns;
  os << "\"" << this->getOriginalName() << "\" " << this->ID << " " << this->TotalCount << " " << nrColumns << std::endl;
  for (auto& col : Columns)
  {
    col->dumpRaw(os);
  }
  os << std::endl;
}

//------------------------------------------------------------------------------
void Table::dumpXml( std::ostream& os )
{
  if( this->Columns.size() == 0 )
    throw generic_error(generic_error::INVALID_TABLE, "");
  size_t nrColumns = this->Columns.size();
  os << "<Table Name=\"" << this->getOriginalName() << "\" ID=\"" << this->ID << "\" NbRows=\"" << this->TotalCount << "\">" << std::endl;
  os << "<Columns>" << std::endl;
  for (auto& col : Columns)
  {
    col->dumpXml(os);
  }
  os << "</Columns>" << std::endl;
  os << "</Table>" << std::endl;
}

}