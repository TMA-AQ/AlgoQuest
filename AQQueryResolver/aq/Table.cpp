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
Table::Table(): 
	ID(0), 
	HasCount(false), 
	TotalCount(0), 
	GroupByApplied(false),
	Partition(NULL),
	OrderByApplied(false),
	NoAnswer(false)
{
	memset(szBuffer, 0, STR_BUF_SIZE);
}

//------------------------------------------------------------------------------
Table::Table( const std::string& name, unsigned int ID, bool _temporary ): 
	ID(ID), 
	HasCount(false), 
	TotalCount(0), 
	GroupByApplied(false), 
	Partition(NULL),
	OrderByApplied(false),
	NoAnswer(false),
  temporary(_temporary)
{
	this->setName( name );
	memset(szBuffer, 0, STR_BUF_SIZE);
}

//------------------------------------------------------------------------------
int Table::getColumnIdx( const std::string& name )
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
void Table::loadFromTableAnswerByColumn(aq::AQMatrix& table, const std::vector<llong>& tableIDs, const std::vector<Column::Ptr>& columnTypes, const TProjectSettings& pSettings, const Base& BaseDesc)
{
	aq::Timer timer;
	
	//
  // is the answer empty ?
	this->NoAnswer = table.getTotalCount() == 0;
	if( this->NoAnswer )
		return;
	if( table.getNbColumn() == 0 )
		this->NoAnswer = true;
		
	std::vector<std::vector<size_t> > mapToUniqueIndex;
	std::vector<std::vector<size_t> > uniqueIndex;

	timer.start();
	table.computeUniqueRow(mapToUniqueIndex, uniqueIndex);
	aq::Logger::getInstance().log(AQ_INFO, "Sorted and Unique Index: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());

	// 
	// For each column, turn numeric references into actual values
	timer.start();
	boost::thread_group grp;
	this->TotalCount = table.getTotalCount();
	for( size_t idx = 0; idx < columnTypes.size(); ++idx )
	{
		aq::Timer realValuesTimer;

		//get table idx for this column
		size_t idxColumn = 0;
		std::string tableName = columnTypes[idx]->getTableName();
		size_t tableID = BaseDesc.getTable(tableName)->ID;
		for( size_t idx2 = 0; idx2 < tableIDs.size(); ++idx2 )
			if( tableID == tableIDs[idx2] )
			{
				idxColumn = idx2;
				break;
			}
		if( idxColumn == tableIDs.size() )
			throw generic_error(generic_error::GENERIC, "");
			
		Column::Ptr newColumn = new Column(*columnTypes[idx]);
		this->Columns.push_back(newColumn);
		boost::thread * th = new boost::thread(boost::bind(&Table::loadColumn, this, newColumn, uniqueIndex[idxColumn], mapToUniqueIndex[idxColumn], columnTypes[idx], pSettings, BaseDesc));
		grp.add_thread(th);
	}
	grp.join_all();
	aq::Logger::getInstance().log(AQ_INFO, "Turn into Real Values: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());

	//
	// Add Count column
	this->HasCount = table.hasCountColumn();
	if( this->HasCount )
	{
		Column::Ptr c(new Column(COL_TYPE_BIG_INT));
		c->setName("Count");
		const std::vector<size_t>& v = table.getCount();
		for(std::vector<size_t>::const_iterator it = v.begin(); it != v.end(); ++it)
		{
			c->Items.push_back(new ColumnItem(static_cast<double>(*it)));
		}
		this->Columns.push_back(c);
	}
}

//------------------------------------------------------------------------------
#if defined(USE_MAP_VIEW_OF_FILE)
void Table::loadColumn(Column::Ptr col, const Table& aqMatrix, const std::vector<size_t>& uniqueIndex, const std::vector<size_t>& mapToUniqueIndex, const Column::Ptr columnType, const TProjectSettings& pSettings, const Base& BaseDesc)
{				
	aq::Timer realValuesTimer;

	//get table idx for this column
	string tableName = columnType->getTableName();
	int tableIdx = BaseDesc.getTableIdx( tableName );
	int tableID = BaseDesc.Tables[tableIdx].ID;
	
	//load the required column values
	llong currentNumPack = -1;
	FILE* prmFile = NULL, *theFile = NULL;

	unsigned char		*pTmpBuf = NULL;
	unsigned int		nTmpBufSize = 0;
	unsigned int		nBinItemSize = 0;

	pTmpBuf			= NULL;
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
	default:
		assert( 0 );
	}

	pTmpBuf = new unsigned char[nTmpBufSize];
	boost::scoped_array<unsigned char> pTmpBufDel( pTmpBuf ); //!!!debug13

	Column::Ptr columnValues = new Column( *columnType );
	for( size_t idx2 = 0; idx2 < uniqueIndex.size(); ++idx2 )
	{
		llong absRaw = uniqueIndex[idx2];
		unsigned int numPack = (size_t) (absRaw / pSettings.packSize);
		llong packOffset = absRaw % pSettings.packSize;
		if( numPack != currentNumPack )
		{
			//load current pack
			if( prmFile )
				fclose( prmFile );
			if( theFile )
				fclose( theFile );

			sprintf( szBuffer, "B001T%.4uC%.4uV01P%.12u", tableID, 
				columnType->ID, numPack );
			string dataPath = pSettings.szRootPath;
			dataPath += "data_orga\\vdg\\data\\";
			dataPath += szBuffer;

			string prmFilePath = dataPath + ".prm";
			string theFilePath = dataPath + ".the";

			prmFile = fopen( prmFilePath.c_str(), "rb" );
			if( !prmFile )
				throw generic_error( generic_error::COULD_NOT_OPEN_FILE, "" );
			theFile = fopen( theFilePath.c_str(), "rb" );
			if( !theFile )
			{
				fclose( prmFile );
				throw generic_error( generic_error::COULD_NOT_OPEN_FILE, "" );
			}
			currentNumPack = numPack;
		}
		int prmFileItemSize = 4;
		fseek( prmFile, (long) packOffset * prmFileItemSize, SEEK_SET );
		int theOffset;
		fread( &theOffset, prmFileItemSize, 1, prmFile );
		fseek( theFile, nBinItemSize*theOffset, SEEK_SET );

		switch( columnType->Type )
		{
		case COL_TYPE_INT:
			{
				int *pnItemData;
				pnItemData = (int*)( pTmpBuf );
				fread( pTmpBuf, 1, nBinItemSize, theFile );
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
				fread( pTmpBuf, 1, nBinItemSize, theFile );
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
				fread( pTmpBuf, 1, nBinItemSize, theFile );
				if( *pdItemData == 'NULL' )
					columnValues->Items.push_back( NULL );
				else
					columnValues->Items.push_back( new ColumnItem( *pdItemData ) );
			}
			break;
		case COL_TYPE_VARCHAR:
			{	
				fread( pTmpBuf, 1, nBinItemSize, theFile );
				pTmpBuf[nBinItemSize] = '\0';
				if( strcmp((char*)pTmpBuf, "NULL") == 0 )
					columnValues->Items.push_back( NULL );
				else
					columnValues->Items.push_back( new ColumnItem( string((char*)pTmpBuf) ) );
			}
			break;
		default:
			assert( 0 );
		}
	}
	aq::Logger::getInstance().log(AQ_INFO, "Turn %u numerics references of column %s into real values: Time Elapsed = %s\n", 
		uniqueIndex.size(),
		columnType->getName().c_str(), aq::Timer::getString(realValuesTimer.getTimeElapsed()).c_str());

	Column::Ptr newColumn = new Column( *columnType );
	for( size_t idx2 = 0; idx2 < mapToUniqueIndex.size(); ++idx2 )
		newColumn->Items.push_back( columnValues->Items[mapToUniqueIndex[idx2]] );

	this->Columns.push_back( newColumn );

	if( prmFile )
		fclose( prmFile );
	if( prmFile )
		fclose( theFile );
}

#else

void Table::loadColumn(Column::Ptr col, const std::vector<size_t>& uniqueIndex, const std::vector<size_t>& mapToUniqueIndex, const Column::Ptr columnType, const TProjectSettings& pSettings, const Base& BaseDesc)
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

#endif

//------------------------------------------------------------------------------
void Table::loadFromAnswerRaw(	const char *filePath, char fieldSeparator, 
								std::vector<llong>& tableIDs, bool add )
{
	char		*pszAnswer = NULL;
	char		*psz = NULL;
	FILE		*pFIn = NULL;
	llong		nVal = 0;
	double		dVal = 0;
	char		*pszBigBuffer = NULL;
	bool		foundFirstLine = false;

	pFIn = fopenUTF8( filePath, "rt" );
	FileCloser fileCloser( pFIn );
	if( !pFIn )
		throw generic_error(generic_error::COULD_NOT_OPEN_FILE, "");

  if (!add)
	  this->Columns.clear();

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
				this->NoAnswer = true;
				return;
			}
			//answer is empty, but this is not an error
			if( fields.size() < 1 )
				return;
			//check for "Count"
			std::string lastField = fields[fields.size() - 1];
			boost::to_upper(lastField);
			boost::trim(lastField);
			if( lastField == "COUNT" )
				this->HasCount = true;
			else
				this->HasCount = false;
			//initialize columns

			for( size_t idx = 0; idx < fields.size(); ++idx )
			{
				Column::Ptr column;
				if (!add)
				{
					column = new Column(COL_TYPE_BIG_INT);
					this->Columns.push_back( column );
				}

				// add or check 'Count'
				if( this->HasCount && (idx + 1 == fields.size()) )
				{
					if (add)
					{
						if (strcmp(fields[idx], "Count") != 0)
							throw generic_error(generic_error::INVALID_TABLE, "");
					}
					else
						column->setName( fields[idx] );
				}
				else // add or check id table
				{
					char* tableNr = strchr(fields[idx], ':');
					std::string tableNrStr( tableNr + 1 );
					boost::trim( tableNrStr );
					llong tableID;
					StrToInt( tableNrStr.c_str(), &tableID );

					if (add)
					{
						if (tableIDs[idx] != tableID)
							throw generic_error(generic_error::INVALID_TABLE, "");
					}
					else
						tableIDs.push_back( tableID );
				}
			}
			continue;
		}
		case 1: //second line is count(*)
      try 
      { 
        this->TotalCount = boost::lexical_cast<uint64_t>(psz); 
      }
      catch (const boost::bad_lexical_cast&) 
      {
        throw aq::generic_error(aq::generic_error::AQ_ENGINE, "invalid aq engine answer file");
      }
			if( this->TotalCount == 0 )
				this->NoAnswer = true;
			continue;
		case 2: //third line is the number of rows in the answer
			continue;
		default:;
		}
		/* Lines must start with a field separator */
		if( psz[0] != fieldSeparator )
			continue;
		if( fields.size() != this->Columns.size() )
			throw generic_error(generic_error::INVALID_TABLE, "");
		assert( fields.size() == this->Columns.size() );
		for( size_t idx = 0; idx < fields.size(); ++idx )
		{
			nVal = 0;
			StrToInt( fields[idx], &nVal );
			this->Columns[idx]->Items.push_back( new ColumnItem( (double) nVal) );
		}
	}
}

//------------------------------------------------------------------------------
int Table::saveToAnswer( const char* filePath, char fieldSeparator, bool answerFormat )
{
	std::vector<size_t> dummy;
	return this->saveToAnswer( filePath, fieldSeparator, dummy, answerFormat );
}

//------------------------------------------------------------------------------
int Table::saveToAnswer(	const char* filePath, char fieldSeparator, 
							std::vector<size_t>& deletedRows, bool answerFormat )
{
	FILE *pFOut = fopen( filePath, "wt" );
	FileCloser fileClose(pFOut);
	char szBuffer[STR_BUF_SIZE];
	memset(szBuffer, 0, STR_BUF_SIZE);

	if ( pFOut == NULL )
		return -1;

	if( this->NoAnswer ||
		this->Columns.size() < 1 || 
		this->Columns[0]->Items.size() < 1 )
	{
		return 0;
	}

	if( answerFormat )
	{
		//write column names
		for( size_t idx = 0; idx < this->Columns.size(); ++idx )
		{
			if( this->Columns[idx]->Invisible )
				continue;
			if( fputs( "; ", pFOut ) < 0 )
				return -1;
			assert( this->Columns[idx] );
			if ( fputs( this->Columns[idx]->getDisplayName().c_str(), pFOut ) < 0 )
				return -1;
		}
		if( fputc( '\n', pFOut ) < 0 )
			return -1;
	}

	size_t nrColumns = this->Columns.size();
	if( !answerFormat && this->HasCount )
		--nrColumns;

	//prepare deleted rows
	sort( deletedRows.begin(), deletedRows.end() );
	size_t deletedIdx = 0;

	//write column values
	for( size_t idx2 = 0; idx2 < this->Columns[0]->Items.size(); ++idx2 )
	{
		//check if we should skip row
		if( deletedIdx < deletedRows.size() )
		{
			if( idx2 > deletedRows[deletedIdx] &&
				(deletedIdx + 1) < deletedRows.size() )
				++deletedIdx;
			if( idx2 == deletedRows[deletedIdx] )
				continue;
		}
		llong count = 1;
		if( !answerFormat && this->HasCount )
			count = (llong) this->Columns[nrColumns]->Items[idx2]->numval;
		for( int idx3 = 0; idx3 < count; ++idx3 )
		{
			for( size_t idx = 0; idx < nrColumns; ++idx )
			{
				if( this->Columns[idx]->Invisible )
					continue;
				if( answerFormat )
				{
					if( fputs( "; ", pFOut ) < 0 )
						return -1;
				}
				else
					if( idx > 0 )
						if( fputs( ";", pFOut ) < 0 )
							return -1;

				ColumnItem* item = this->Columns[idx]->Items[idx2].get();
				if( !item )
				{
					if( sprintf( szBuffer, "NULL" ) < 0 )
						return -1;
				}
				else
					item->toString( szBuffer, this->Columns[idx]->Type );
				
				fputs( szBuffer, pFOut );
			}
			if( fputc( '\n', pFOut ) < 0 )
				return -1;
		}
	}
	return 0;
}

//------------------------------------------------------------------------------
std::vector<Column::Ptr> Table::getColumnsByName( std::vector<Column::Ptr>& columns )
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
std::vector<Column::Ptr> Table::getColumnsTemplate()
{
	std::vector<Column::Ptr> newColumns;
	for( size_t idx = 0; idx < this->Columns.size(); ++idx )
	{
		Column& col = *this->Columns[idx].get();
		Column::Ptr newColumn = new Column( col );
		newColumn->Invisible = col.Invisible;
		newColumn->GroupBy = col.GroupBy;
		newColumns.push_back( newColumn );
	}
	return newColumns;
}

//------------------------------------------------------------------------------
void Table::updateColumnsContent( const std::vector<Column::Ptr>& newColumns )
{
	if( newColumns.size() > this->Columns.size() )
		throw generic_error( generic_error::GENERIC, "" );
	this->Columns.resize( newColumns.size() );
	if( newColumns.size() == 0 )
		return;
	if( this->HasCount )
	{
		this->TotalCount = 0;
		Column::Ptr count = newColumns[newColumns.size() - 1];
		for( size_t idx = 0; idx < count->Items.size(); ++idx )
			this->TotalCount += (int) count->Items[idx]->numval;
	}
	else
		this->TotalCount = newColumns[0]->Items.size();
	for( size_t idx = 0; idx < newColumns.size(); ++idx )
	{
		this->Columns[idx]->Items.clear();
		this->Columns[idx]->Items = newColumns[idx]->Items;
	}
}

//------------------------------------------------------------------------------
void Table::unravel( aq::verb::TablePartition::Ptr partition )
{
	if( !this->HasCount )
		return;
	if( this->Columns.size() < 2 )
		return;
	if( this->TotalCount == this->Columns[0]->Items.size() )
		return;
	std::vector<Column::Ptr> newColumns = this->getColumnsTemplate();
	Column::Ptr count = this->Columns[this->Columns.size() - 1];

	size_t rowIdx = 0;
	size_t partitionRowIdx = 0;
	for( size_t idx = 0; idx < this->Columns[0]->Items.size(); ++idx )
	{
		size_t nrItems = (size_t) count->Items[idx]->numval;
		rowIdx = rowIdx + nrItems;
		if( idx + 1 == partition->Rows[partitionRowIdx+1] )
		{
			partition->Rows[partitionRowIdx+1] = rowIdx;
			++partitionRowIdx;
		}
		for( size_t idx2 = 0; idx2 < this->Columns.size()-1; ++idx2 )
			for( size_t idx3 = 0; idx3 < nrItems; ++idx3 )
				newColumns[idx2]->Items.push_back( this->Columns[idx2]->Items[idx] );
	}
	Column::Ptr newCount = newColumns[newColumns.size() - 1];
	for( size_t idx = 0; idx < newColumns[0]->Items.size(); ++idx )
		newCount->Items.push_back( new ColumnItem(1) );

	this->updateColumnsContent( newColumns );
}

//------------------------------------------------------------------------------
void Table::cleanRedundantColumns()
{
	std::vector<Column::Ptr> newColumns;
	for( size_t idx = 0; idx < this->Columns.size(); ++idx )
		if( !this->Columns[idx]->Invisible ||
			this->Columns[idx]->GroupBy || 
			this->Columns[idx]->OrderBy )
			newColumns.push_back( this->Columns[idx] );
	this->Columns = newColumns;
}

//------------------------------------------------------------------------------
void Table::groupBy()
{
	if( this->Columns.size() == 0 ||
		this->HasCount && this->Columns.size() == 1 ||
		this->OrderByApplied )
		return;

	std::vector<Column::Ptr> columns;
	this->orderBy( columns, NULL );
	
	//detect duplicates
	size_t size = this->Columns[0]->Items.size();
	size_t nrColumns = this->Columns.size();
	if( this->HasCount )
		--nrColumns;
	std::vector<bool> duplicates;
	duplicates.push_back( false );
	for( size_t idx = 1; idx < size; ++idx )
	{
		bool duplicate = true;
		for( size_t idx2 = 0; idx2 < nrColumns; ++idx2 )
			if( !ColumnItem::equal(	this->Columns[idx2]->Items[idx].get(), 
						this->Columns[idx2]->Items[idx - 1].get(),
						this->Columns[idx2]->Type ) )
			{
				duplicate = false;
				break;
			}
		duplicates.push_back( duplicate );
	}

	//recreate table
	std::vector<Column::Ptr> newColumns = this->getColumnsTemplate();

	//create new grouped table
	Column::Ptr oldCount = NULL;
	Column::Ptr newCount = NULL;
	if( this->HasCount )
	{
		oldCount = this->Columns[this->Columns.size() - 1];
		newCount = newColumns[newColumns.size() - 1];
	}

	int nrNew = -1;
	for( size_t idx = 0; idx < size; ++idx )
		if( duplicates[idx] )
		{
			if( oldCount )
				newCount->Items[nrNew]->numval += oldCount->Items[idx]->numval;
		}
		else
		{
			for( size_t idx2 = 0; idx2 < this->Columns.size(); ++idx2 )
				newColumns[idx2]->Items.push_back( this->Columns[idx2]->Items[idx] );
			++nrNew;
		}

	this->updateColumnsContent( newColumns );
}

//------------------------------------------------------------------------------
void Table::orderBy(	std::vector<Column::Ptr> columns, 
						aq::verb::TablePartition::Ptr partition )
{
	std::vector<size_t> index;
	this->orderBy( columns, partition, index );
}

//------------------------------------------------------------------------------
void Table::orderBy(	std::vector<Column::Ptr> columns, 
						aq::verb::TablePartition::Ptr partition,
						std::vector<size_t>& index )
{
	aq::Timer timer;

	if( this->Columns.size() < 1 || this->Columns[0]->Items.size() < 2 )
		return; //nothing to sort

	if( columns.size() == 0 )
		columns = this->Columns;
	else
		columns = this->getColumnsByName( columns );

	//index = 0, 1, 2, 3 ..
	index.resize( this->Columns[0]->Items.size() );
	for( size_t idx = 0; idx < index.size(); ++idx )
		index[idx] = idx;

	if( !partition )
		partition = new aq::verb::TablePartition();

	std::vector<size_t>& partitions = partition->Rows;
	if( partitions.size() == 0 )
	{
		partitions.push_back( 0 );
		partitions.push_back( index.size() );
	}
	std::vector<size_t> oldPartitions;

	timer.start();
	for( size_t idx = 0; idx < columns.size(); ++idx )
	{
		Column::inner_column_cmp_t cmp(*(columns[idx].get()));

		oldPartitions = partitions;
		partitions.clear();

		assert( oldPartitions.size() > 1 );
		for( size_t idx2 = 0; idx2 < oldPartitions.size() - 1; ++idx2 )
		{
			size_t pos1 = oldPartitions[idx2];
			size_t pos2 = oldPartitions[idx2 + 1];
			assert( pos1 < pos2 );
			sort( index.begin() + pos1, index.begin() + pos2, cmp);
			//detect new partitions within this old partition
			partitions.push_back( pos1 );
			for( size_t idx3 = pos1 + 1; idx3 < pos2; ++idx3 )
				if( !ColumnItem::equal(	columns[idx]->Items[index[idx3 - 1]].get(),
							columns[idx]->Items[index[idx3]].get(),
							columns[idx]->Type ) )
					partitions.push_back( idx3 );
		}
		partitions.push_back( index.size() );
	}
	aq::Logger::getInstance().log(AQ_INFO, "generate ordered indexes: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());

	//recreate table
	timer.start();
	std::vector<Column::Ptr> newColumns = this->getColumnsTemplate();
	for( size_t idx = 0; idx < index.size(); ++idx )
		for( size_t idx2 = 0; idx2 < this->Columns.size(); ++idx2 )
			newColumns[idx2]->Items.push_back( this->Columns[idx2]->Items[index[idx]] );
	aq::Logger::getInstance().log(AQ_INFO, "Recreate Table: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
			
	timer.start();
	this->updateColumnsContent( newColumns );
	aq::Logger::getInstance().log(AQ_INFO, "Update ColumnsContent: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
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