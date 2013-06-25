#include "Base.h"
#include <aq/Exceptions.h>
#include <boost/bind.hpp>
#include <boost/scoped_array.hpp>

namespace aq
{
  
//------------------------------------------------------------------------------
char* SkipWhiteSpaces( char* pszStr ) {
	if ( pszStr == NULL )
		return NULL;
	while ( *pszStr == ' ' || *pszStr == '\t' || *pszStr == '\r' || *pszStr == '\n' )
		pszStr++;
	return pszStr;
}

//------------------------------------------------------------------------------
char* SkipToNextLine_sFirstChar( char* pszStr ) {
	if ( pszStr == NULL )
		return NULL;
	while ( *pszStr != '\0' && *pszStr != '\r' && *pszStr != '\n' )
		pszStr++;
	return SkipWhiteSpaces( pszStr );
}

//------------------------------------------------------------------------------
// Return pointer to string after the terminating Quotation mark ! Or NULL on error !
char* ExtractStringFromQuotes( char* pszStr, char* pszExtractedStr, unsigned int cbExtractedStr ) {
	char *pszTmp;
	size_t nLen;

	if ( pszExtractedStr == NULL )
		return NULL;

	if ( pszStr == NULL || *pszStr == '\0' || *pszStr != '"' )
		return NULL;

	/* Skip " */
	pszStr++;

	/* Find the name end - terminator is " */
	pszTmp = strchr( pszStr, '"' );
	if ( pszTmp == NULL )
		return NULL;

	nLen = pszTmp - pszStr;

	/* Nothing to copy */
	if ( nLen == 0 )
		return NULL;

	/* Check if enough space in the return buffer */
	if ( cbExtractedStr < nLen )
		return NULL;

	*pszExtractedStr = '\0';
	strncpy( pszExtractedStr, pszStr, nLen );
	pszExtractedStr[ nLen ] = '\0';

	return pszTmp + 1;
}

//------------------------------------------------------------------------------
Table::Ptr Base::getTable(size_t id)
{
	for( size_t i = 0; i < this->Tables.size(); ++i )
		if( id == this->Tables[i]->ID )
			return this->Tables[i];
	throw generic_error(generic_error::INVALID_TABLE, "cannot find table %u", id);
}

//------------------------------------------------------------------------------
const Table::Ptr Base::getTable(size_t id) const
{
	return const_cast<Base*>(this)->getTable(id);
}

//------------------------------------------------------------------------------
Table::Ptr Base::getTable( const std::string& name )
{
	std::string auxName = name;
	strtoupr( auxName );
	aq::Trim( auxName );
	for( size_t idx = 0; idx < this->Tables.size(); ++idx )
  {
		if( auxName == this->Tables[idx]->getName() )
    {
      if (this->Tables[idx]->getReferenceTable() != "")
      {
        return this->getTable(this->Tables[idx]->getReferenceTable());
      }
      else
      {
        return this->Tables[idx];
      }
    }
  }
	throw generic_error(generic_error::INVALID_TABLE, "cannot find table %s", name.c_str());
}

//------------------------------------------------------------------------------
const Table::Ptr Base::getTable( const std::string& name ) const
{
  return const_cast<Base*>(this)->getTable(name);
}

//------------------------------------------------------------------------------
void Base::loadFromBaseDesc(const aq::base_t& base) 
{
  this->Name = base.nom;
  std::for_each(base.table.begin(), base.table.end(), [&] (const base_t::table_t& table) {
		Table::Ptr pTD(new Table(table.nom, table.num));
		pTD->TotalCount = table.nb_enreg;
    std::for_each(table.colonne.begin(), table.colonne.end(), [&] (const base_t::table_t::col_t& column) {
      aq::ColumnType type = aq::symbole_to_column_type(column.type);
      unsigned int size = 0;
      switch (type)
      {
      case COL_TYPE_VARCHAR: size = column.taille * sizeof(char); break;
      case COL_TYPE_INT: size = 4; break;
      case COL_TYPE_BIG_INT:
      case COL_TYPE_DOUBLE:
      case COL_TYPE_DATE1:
      case COL_TYPE_DATE2:
      case COL_TYPE_DATE3:
      case COL_TYPE_DATE4: size = 8; break;
      }
      pTD->Columns.push_back(new Column(column.nom, column.num, size, type));
		});
		this->Tables.push_back(pTD);
  });
}

//------------------------------------------------------------------------------
void Base::loadFromRawFile( const char* pszDataBaseFile ) {
	unsigned int nTableCount;
	unsigned int nColumnCount;
	unsigned int nTableRecordsNb;
	unsigned int iTable, iColumn;
	char *pszStr;

	char szBaseName[ 1000 ];
	/* unsigned int nBaseId; */
	char szTableName[ 1000 ];
	unsigned int nTableId;
	char szColumnName[ 1000 ];
	unsigned int nColumnId;
	unsigned int nColumnSize;
	char szColumnType[ 100 ];
	ColumnType eColumnType;

	/* Load DataBase Structure */
	char* pszDataBaseDef = LoadFile( pszDataBaseFile );
	if ( pszDataBaseDef == NULL )
		throw generic_error(generic_error::INVALID_BASE_FILE, "");
	boost::scoped_array<char> pszDataBaseDefDel( pszDataBaseDef );

	/* Get DataBase Name */
	pszStr = SkipWhiteSpaces( pszDataBaseDef );
	if ( pszStr == NULL || *pszStr == '\0' )
		throw generic_error(generic_error::INVALID_BASE_FILE, "");
	if ( sscanf( pszStr, "%s", szBaseName ) != 1 )
		throw generic_error(generic_error::INVALID_BASE_FILE, "");
	this->Name = szBaseName;

	/* Get Table Count */
	pszStr = SkipToNextLine_sFirstChar( pszStr );
	if ( pszStr == NULL || *pszStr == '\0' )
		throw generic_error(generic_error::INVALID_BASE_FILE, "");
	if ( sscanf( pszStr, "%u", &nTableCount ) != 1 )
		throw generic_error(generic_error::INVALID_BASE_FILE, "");
	if ( nTableCount == 0 )
		throw generic_error(generic_error::INVALID_BASE_FILE, "");

	for ( iTable = 0; iTable < nTableCount; iTable++ ) {

		pszStr = SkipToNextLine_sFirstChar( pszStr );
		if ( pszStr == NULL || *pszStr == '\0' || *pszStr != '"' ) 
			throw generic_error(generic_error::INVALID_BASE_FILE, "");

		pszStr = ExtractStringFromQuotes( pszStr, szTableName, sizeof( szTableName ) );
		if ( pszStr == NULL || *pszStr == '\0' ) 
			throw generic_error(generic_error::INVALID_BASE_FILE, "");

		if ( sscanf( pszStr, "%u %u %u", &nTableId, &nTableRecordsNb, &nColumnCount ) != 3 ) 
			throw generic_error(generic_error::INVALID_BASE_FILE, "");

		// Add Table
		Table::Ptr pTD(new Table(std::string(szTableName), nTableId));
		pTD->TotalCount = nTableRecordsNb;
		for ( iColumn = 0; iColumn < nColumnCount; iColumn++ ) {
			pszStr = SkipToNextLine_sFirstChar( pszStr );
			if ( pszStr == NULL || *pszStr == '\0' || *pszStr != '"' ) 
				throw generic_error(generic_error::INVALID_BASE_FILE, "");

			pszStr = ExtractStringFromQuotes( pszStr, szColumnName, sizeof( szColumnName ) );
			if ( pszStr == NULL || *pszStr == '\0' ) 
				throw generic_error(generic_error::INVALID_BASE_FILE, "");

			if ( sscanf( pszStr, "%u %u %s", &nColumnId, &nColumnSize, szColumnType ) != 3 ) 
				throw generic_error(generic_error::INVALID_BASE_FILE, "");

			strtoupr( szColumnType );
			if ( strcmp( szColumnType, "NUMBER" ) == 0 )
      {
        nColumnSize = 8;
				eColumnType = COL_TYPE_INT;
      }
			else if ( strcmp( szColumnType, "INT" ) == 0 )
      {
        nColumnSize = 4;
				eColumnType = COL_TYPE_INT;
      }
			else if ( strcmp( szColumnType, "BIG_INT" ) == 0 )
      {
        nColumnSize = 8;
				eColumnType = COL_TYPE_BIG_INT;
      }
			else if ( strcmp( szColumnType, "FLOAT" ) == 0 )
      {
        nColumnSize = 8;
				eColumnType = COL_TYPE_DOUBLE;
      }
			else if ( strcmp( szColumnType, "DOUBLE" ) == 0 )
      {
        nColumnSize = 8;
				eColumnType = COL_TYPE_DOUBLE;
      }
			else if ( strcmp( szColumnType, "DATE1" ) == 0 )
      {
        nColumnSize = 8;
				eColumnType = COL_TYPE_DATE1;
      }
			else if ( strcmp( szColumnType, "DATE2" ) == 0 )
      {
        nColumnSize = 8;
				eColumnType = COL_TYPE_DATE2;
      }
			else if ( strcmp( szColumnType, "DATE3" ) == 0 )
      {
        nColumnSize = 8;
				eColumnType = COL_TYPE_DATE3;
      }
			else // if ( strcmp( szColumnType, "VARCHAR2" ) == 0 )	// Same for CHAR
      {
        nColumnSize *= sizeof(char);
				eColumnType = COL_TYPE_VARCHAR;
      }

			pTD->Columns.push_back( new Column( std::string(szColumnName), nColumnId, nColumnSize, eColumnType ) );
		}

		this->Tables.push_back( pTD );
	}
}

//------------------------------------------------------------------------------
void Base::saveToRawFile( const char* pszDataBaseFile )
{
	FILE *pFOut = fopen( pszDataBaseFile, "wt" );
	FileCloser fileClose(pFOut);
	if ( pFOut == NULL )
		throw generic_error(generic_error::COULD_NOT_OPEN_FILE, "");

	fputs( this->Name.c_str(), pFOut );
	fprintf( pFOut, "\n%u\n", this->Tables.size() );
	for( size_t idx = 0; idx < this->Tables.size(); ++idx )
	{
		Table& table = *this->Tables[idx];
		if( table.Columns.size() == 0 )
			throw generic_error(generic_error::INVALID_TABLE, "");
		size_t nrColumns = table.Columns.size();
		if( table.HasCount )
			--nrColumns;
		fprintf( pFOut, "\n\"%s\" %u %lld %u\n", table.getOriginalName().c_str(), table.ID, 
			table.TotalCount, nrColumns );
		for( size_t idx2 = 0; idx2 < nrColumns; ++idx2 )
		{
			Column& col = *table.Columns[idx2];
			col.ID = idx2 + 1;
			std::string colName = col.getOriginalName();
			size_t pos = colName.find('.');
			if( pos != std::string::npos )
				colName = colName.substr( pos + 1 );
			fprintf( pFOut, "\"%s\" %u %u ", colName.c_str(), col.ID, col.Size );
			char* szColumnType = NULL;
			switch( col.Type )
			{
			case COL_TYPE_INT: szColumnType = "INT"; break;
			case COL_TYPE_BIG_INT: szColumnType = "BIG_INT"; break;
			case COL_TYPE_DOUBLE: szColumnType = "DOUBLE"; break;
			case COL_TYPE_DATE1: szColumnType = "DATE1"; break;
			case COL_TYPE_DATE2: szColumnType = "DATE2"; break;
			case COL_TYPE_DATE3: szColumnType = "DATE3"; break;
			case COL_TYPE_VARCHAR: szColumnType = "VARCHAR2"; break;
			default:
				throw generic_error(generic_error::NOT_IMPLEMENED, "");
			}
			fprintf( pFOut, "%s\n", szColumnType );
		}
	}
}

//------------------------------------------------------------------------------
void Base::dumpRaw( std::ostream& os )
{
  os << this->Name << std::endl;
  os << this->Tables.size() << std::endl << std::endl;
  std::for_each(this->Tables.begin(), this->Tables.end(), boost::bind(&Table::dumpRaw, _1, boost::ref(os)));
 }
 
//------------------------------------------------------------------------------
void Base::dumpXml( std::ostream& os )
{
  os << "<Database Name=\"" << this->Name << "\">" << std::endl;
  os << "<Tables>" << std::endl;
  std::for_each(this->Tables.begin(), this->Tables.end(), boost::bind(&Table::dumpXml, _1, boost::ref(os)));
  os << "</Tables>" << std::endl;
  os << "</Database>" << std::endl;
 }
 
}