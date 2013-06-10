#include "Column.h"
#include <aq/Utilities.h>
#include <aq/Exceptions.h>
#include <boost/scoped_array.hpp>

namespace aq
{

//-----------------------------------------------------------------------------
Column::inner_column_cmp_t::inner_column_cmp_t(Column& lessThanColumn)
	: m_lessThanColumn(lessThanColumn)
{
}
bool Column::inner_column_cmp_t::operator()(size_t idx1, size_t idx2)
{
	return ColumnItem::lessThan(m_lessThanColumn.Items[idx1].get(), m_lessThanColumn.Items[idx2].get(), m_lessThanColumn.Type);
}

Column::inner_column_cmp_2_t::inner_column_cmp_2_t(Column& lessThanColumn)
	: m_lessThanColumn(lessThanColumn)
{
}
bool Column::inner_column_cmp_2_t::operator()(ColumnItem::Ptr item1, ColumnItem::Ptr item2)
{
	return ColumnItem::lessThan(item1.get(), item2.get(), m_lessThanColumn.Type );
}


//------------------------------------------------------------------------------
Column::Column()
	:	
	ID(0), 
	Size(0), 
	Type(COL_TYPE_INT), 
	prmFileItemSize(4),
	currentNumPack(-1),
	packOffset(0),
	nBinItemSize(0),
	Invisible(false), 
	GroupBy(false), 
	OrderBy(false),
  Temporary(false)
{
	this->setBinItemSize();
}

//------------------------------------------------------------------------------
Column::Column(	const std::string& name, unsigned int ID, unsigned int size, ColumnType type)
	: 
	ID(ID), 
	Size(size), 
	Type(type),
	prmFileItemSize(4),
	currentNumPack(-1),
	packOffset(0),
	nBinItemSize(0),
	Invisible(false), 
	GroupBy(false), 
	OrderBy(false),
  Temporary(false)
{
	this->setBinItemSize();
	this->setName( name );
}

//------------------------------------------------------------------------------
Column::Column( ColumnType type )
	: 
	ID(0),
	Size(0),
	Type(type),
	prmFileItemSize(4),
	currentNumPack(-1),
	packOffset(0),
	nBinItemSize(0),
	Invisible(false), 
	GroupBy(false),
	OrderBy(false),
  Temporary(false)
{
	this->setBinItemSize();
}

//------------------------------------------------------------------------------
Column::Column( const Column& source )
	:
	ID(source.ID),
	Size(source.Size),
	Type(source.Type),
	Invisible(false),
	GroupBy(false),
	OrderBy(false),
  Temporary(source.Temporary),
	Name(source.Name),
	OriginalName(source.OriginalName),
	DisplayName(source.DisplayName),
	Count(source.Count),
	TableName(source.TableName),
	prmFileItemSize(source.prmFileItemSize),
	currentNumPack(source.currentNumPack),
	packOffset(source.packOffset),
	nBinItemSize(source.nBinItemSize)
{
	this->setBinItemSize();
}

//----------------------------------------------------------------------------
Column::~Column() 
{
}

//------------------------------------------------------------------------------
Column& Column::operator=(const Column& source)
{
	if (this != &source)
	{
		this->ID = source.ID;
		this->Size = source.Size;
		this->Type = source.Type;
		this->Invisible = false;
		this->GroupBy = false;
		this->OrderBy = false;
    this->Temporary = source.Temporary;
		this->Name = source.Name;
		this->OriginalName = source.OriginalName;
		this->DisplayName = source.DisplayName;
		this->Count = source.Count;
		this->TableName = source.TableName;
		this->prmFileItemSize = source.prmFileItemSize;
		this->currentNumPack = source.currentNumPack,
		this->packOffset = source.packOffset;
		this->nBinItemSize = source.nBinItemSize;
		this->setBinItemSize();
	}
	return *this;
}

void Column::setBinItemSize()
{
	switch( this->Type )
	{
	case COL_TYPE_INT:
		nBinItemSize	= 4;
		break;
	case COL_TYPE_BIG_INT:
	case COL_TYPE_DATE1:
	case COL_TYPE_DATE2:
	case COL_TYPE_DATE3:
	case COL_TYPE_DOUBLE:
		nBinItemSize	= 8;
		break;
	case COL_TYPE_VARCHAR:
		nBinItemSize	= this->Size;
		break;
	}
}

//------------------------------------------------------------------------------
void Column::setName( const std::string& name )
{
	this->OriginalName = name;
	this->Name = name;
	strtoupr( this->Name );
	Trim( this->Name );
}

//------------------------------------------------------------------------------
void Column::setDisplayName( const std::string& name )
{
	this->DisplayName = name;
}

//------------------------------------------------------------------------------
std::string& Column::getName()
{
	return this->Name;
}

//------------------------------------------------------------------------------
std::string& Column::getOriginalName()
{
	return this->OriginalName;
}


//------------------------------------------------------------------------------
std::string& Column::getDisplayName()
{
	if( this->DisplayName == "" )
		if( this->OriginalName == "" )
			return this->Name;
		else
			return this->OriginalName;
	else
		return this->DisplayName;
}

//------------------------------------------------------------------------------
void Column::setTableName( const std::string& name )
{
	this->TableName = name;
	strtoupr( this->TableName );
	Trim( this->TableName );
}

//------------------------------------------------------------------------------
std::string& Column::getTableName()
{
	return this->TableName;
}

//------------------------------------------------------------------------------
void big_endian_to_little_endian( unsigned int *pnVal ) {
	unsigned char nTmp;
	unsigned char *pTmp;

	if ( pnVal == NULL )
		return;

	pTmp = (unsigned char*)pnVal;

	nTmp = pTmp[ 0 ];
	pTmp[ 0 ] = pTmp[ 3 ];
	pTmp[ 3 ] = nTmp;

	nTmp = pTmp[ 1 ];
	pTmp[ 1 ] = pTmp[ 2 ];
	pTmp[ 2 ] = nTmp;
}

//------------------------------------------------------------------------------
/* Return NULL on error */
int Column::loadFromThesaurus(	const char *pszFilePath, int nFileType, 
								unsigned int nColumnSize, ColumnType eColumnType, 
								int *pErr ) {
	FILE				*pFIn;
	long				nFileSize;
	unsigned int		nItemCount, i;
	char				*psz;
	unsigned char		*pLineBuf;
	unsigned char		*pTmpBuf;
	unsigned int		nTmpBufSize;
	unsigned int		nBinItemSize;


	pTmpBuf			= NULL;
	nBinItemSize	= 0;
	if ( eColumnType == COL_TYPE_INT ) {
		nTmpBufSize		= 1000;
		nBinItemSize	= 4;
	} else if ( eColumnType == COL_TYPE_BIG_INT || 
				eColumnType == COL_TYPE_DATE1 || 
				eColumnType == COL_TYPE_DATE2 || 
				eColumnType == COL_TYPE_DATE3 || 
				eColumnType == COL_TYPE_DOUBLE ) {
		nTmpBufSize		= 1000;
		nBinItemSize	= 8;
	} else {
		nTmpBufSize		= nColumnSize + 1000;
		nBinItemSize	= nColumnSize;
	}

	pTmpBuf = new unsigned char[nTmpBufSize];
	if ( pTmpBuf == NULL ) {
		if ( pErr != NULL )
			*pErr = EXPR_TR_ERR_NOT_ENOUGH_MEMORY;
		return -1;
	}
	boost::scoped_array<unsigned char> pTmpBufDel( pTmpBuf );

	pLineBuf = new unsigned char[nTmpBufSize];
	if ( pLineBuf == NULL ) {
		if ( pErr != NULL )
			*pErr = EXPR_TR_ERR_NOT_ENOUGH_MEMORY;
		return -1;
	}
	boost::scoped_array<unsigned char> pLineBufDel( pLineBuf );

	/* Set the Column Type */
	this->Type = eColumnType;
	if( this->Type == COL_TYPE_BIG_INT )
		this->Type = COL_TYPE_INT;

	/* Open in binary mode */
	pFIn = fopen( pszFilePath, "rb" );
	FileCloser fileCloser(pFIn);
	if ( pFIn == NULL ) {
		if ( pErr != NULL )
			*pErr = EXPR_TR_ERR_THESAURUS_FILE_NOT_FOUND;
		return -1;
	}

	/* Get the file size */
	if ( fseek( pFIn, 0, SEEK_END ) != 0 ) {
		if ( pErr != NULL )
			*pErr = EXPR_TR_ERR_READING_THESAURUS_FILE;
		return -1;
	}

	nFileSize = ftell( pFIn );
	if ( nFileSize == -1 ) {
		if ( pErr != NULL )
			*pErr = EXPR_TR_ERR_INVALID_THESAURUS_FILE;
		return -1;
	}

	/* Seek back to the beginning of the file */
	if ( fseek( pFIn, 0, SEEK_SET ) != 0 ) {
		if ( pErr != NULL )
			*pErr = EXPR_TR_ERR_READING_THESAURUS_FILE;
		return -1;
	}

	if ( nFileType == 0 ) {
		/* Binary File */

		if ( eColumnType == COL_TYPE_INT ) {
			/* Define the Data pointer in our buffer ! */
			int *pnItemData;
			pnItemData = (int*)( pTmpBuf );

			nItemCount	= (unsigned int)nFileSize / nBinItemSize;
			for ( i = 0; i < nItemCount; i++ ) {
				if ( fread( pTmpBuf, 1, nBinItemSize, pFIn ) != nBinItemSize ) {
					if ( pErr != NULL )
						*pErr = EXPR_TR_ERR_READING_THESAURUS_FILE;
					return -1;
				}

				this->Items.push_back( new ColumnItem( *pnItemData ) );
			}
		} else if ( eColumnType == COL_TYPE_DATE1 || eColumnType == COL_TYPE_DATE2 || 
			eColumnType == COL_TYPE_DATE3 || eColumnType == COL_TYPE_BIG_INT ) {
			/* Define the Data pointer in our buffer ! */
			long long *pnItemData;
			pnItemData = (long long*)( pTmpBuf );

			nItemCount	= (unsigned int)nFileSize / nBinItemSize;
			for ( i = 0; i < nItemCount; i++ ) {
				if ( fread( pTmpBuf, 1, nBinItemSize, pFIn ) != nBinItemSize ) {
					if ( pErr != NULL )
						*pErr = EXPR_TR_ERR_READING_THESAURUS_FILE;
					return -1;
				}

				this->Items.push_back( new ColumnItem( (double) *pnItemData ) );
			}
		} else if ( eColumnType == COL_TYPE_DOUBLE ) {
			/* Define the Data pointer in our buffer ! */
			double *pdItemData;
			pdItemData = (double*)( pTmpBuf );

			nItemCount	= (unsigned int)nFileSize / nBinItemSize;
			for ( i = 0; i < nItemCount; i++ ) {
				if ( fread( pTmpBuf, 1, nBinItemSize, pFIn ) != nBinItemSize ) {
					if ( pErr != NULL )
						*pErr = EXPR_TR_ERR_READING_THESAURUS_FILE;
					return -1;
				}

				this->Items.push_back( new ColumnItem( *pdItemData ) );
			}
		} else { // COL_TYPE_VARCHAR
			nItemCount = (unsigned int)nFileSize / nBinItemSize;
			for ( i = 0; i < nItemCount; i++ ) {
				if ( fread( pTmpBuf, 1, nBinItemSize, pFIn ) != nBinItemSize ) {
					if ( pErr != NULL )
						*pErr = EXPR_TR_ERR_READING_THESAURUS_FILE;
					return -1;
				}

				pTmpBuf[ nColumnSize ] = '\0';	// Not -1 !
				this->Items.push_back( new ColumnItem((char*)pTmpBuf) );
			}
		}
	} else {
		/* Text File */
		FILE *pFIn; //redeclare so that FileCloser works correctly

		/* Open in text mode */
		pFIn = fopenUTF8( pszFilePath, "rt" );
		FileCloser fileCloser( pFIn );
		if ( pFIn == NULL ) {
			if ( pErr != NULL )
				*pErr = EXPR_TR_ERR_THESAURUS_FILE_NOT_FOUND;
			return -1;
		}

		while ( fgets( (char*)pLineBuf, nTmpBufSize, pFIn ) != NULL ) {
			/* Skip whitespace from the begin of the line ! */
			psz = (char*)pLineBuf;
			while ( *psz != '\0' && ( *psz == ' ' || *psz == '\t' ) )
				psz++;

			/* Check if valid line ! */
			if ( *psz == '\0' || *psz == '\n' || *psz == '\r' ) {
				pLineBuf[ 0 ] = '\0';
				continue;
			}

			if ( sscanf( psz, "%s", pTmpBuf ) != 1 ) {
				if ( pErr != NULL )
					*pErr = EXPR_TR_ERR_INVALID_THESAURUS_FILE;
				return -1;
			}

			this->Items.push_back( new ColumnItem((char*)pTmpBuf) );
		}
	}

	if ( this->Items.size() == 0 ) {
		if ( pErr != NULL )
			*pErr = EXPR_TR_ERR_INVALID_THESAURUS_FILE;
		return -1;
	}

	//debug13 - for date type, the items aren't sorted for some reason
	inner_column_cmp_2_t cmp(*this);
	sort( this->Items.begin(), this->Items.end(), cmp );

	return 0;
}

//------------------------------------------------------------------------------
void Column::loadFromTmp(ColumnType eColumnType, aq::TemporaryColumnMapper::Ptr colMapper)
{
  this->Type = eColumnType;
  this->Size = 0;

  ColumnItem::Ptr item = 0;
  do
  {
    item = colMapper->loadValue(this->Size++);
    if (item != 0) 
      this->Items.push_back(item);
  } while (item != 0) ;
  
	inner_column_cmp_2_t cmp(*this);
	sort( this->Items.begin(), this->Items.end(), cmp );

}

//------------------------------------------------------------------------------
void Column::increase( size_t newSize )
{
	assert( this->Items.size() > 0 );
	for( size_t idx = this->Items.size() ; idx < newSize; ++idx )
		this->Items.push_back( this->Items[0] );
}

//------------------------------------------------------------------------------
void Column::setCount( Column::Ptr count )
{
	if( count->Type != COL_TYPE_BIG_INT )
		throw generic_error(generic_error::TYPE_MISMATCH, 
			"Count column set to type other than INT.");
	this->Count = count;
}

//------------------------------------------------------------------------------
Column::Ptr Column::getCount()
{
	return this->Count;
}

//------------------------------------------------------------------------------
void Column::loadFromFile( const std::string& file )
{
	FILE				*pFIn;
	size_t           nFileSize;
	size_t           nItemCount;
	unsigned char		*pLineBuf;
	unsigned char		*pTmpBuf;
	size_t           nTmpBufSize;
	size_t           nBinItemSize;
	char				ch;

	pTmpBuf			= NULL;
	nBinItemSize	= 0;
	switch( this->Type )
	{
	case COL_TYPE_INT:
		nTmpBufSize		= 1000;
		nBinItemSize	= 4;
		break;
	case COL_TYPE_BIG_INT:
	case COL_TYPE_DATE1:
	case COL_TYPE_DATE2:
	case COL_TYPE_DATE3:
	case COL_TYPE_DOUBLE:
		nTmpBufSize		= 1000;
		nBinItemSize	= 8;
		break;
	case COL_TYPE_VARCHAR:
		nTmpBufSize		= this->Size + 1000;	// reserve 1000 bytes extra space for text mode !
		nBinItemSize	= this->Size;
		break;
	default:
		assert( 0 );
	}

	pTmpBuf = new unsigned char[nTmpBufSize];
	boost::scoped_array<unsigned char> pTmpBufDel( pTmpBuf ); //!!!debug13

	pLineBuf = new unsigned char[nTmpBufSize];
	boost::scoped_array<unsigned char> pLineBufDel( pLineBuf ); //!!!debug13

	/* Open in binary mode */
	pFIn = fopenUTF8( file.c_str(), "rb" );
	FileCloser fileCloser(pFIn);

	fseek( pFIn, 0, SEEK_END );
	nFileSize = ftell( pFIn );
	fseek( pFIn, 0, SEEK_SET );
	nItemCount	= (unsigned int)nFileSize / nBinItemSize;

	switch( this->Type )
	{
	case COL_TYPE_INT:
		int *pnItemData;
		pnItemData = (int*)( pTmpBuf );
		for( size_t idx = 0; idx < nItemCount; ++idx ) 
		{
			fread( pTmpBuf, 1, nBinItemSize, pFIn );
			this->Items.push_back( new ColumnItem( *pnItemData ) );
		}
		break;
	case COL_TYPE_BIG_INT:
	case COL_TYPE_DATE1:
	case COL_TYPE_DATE2:
	case COL_TYPE_DATE3:
		{
			long long *pnItemData;
			pnItemData = (long long*)( pTmpBuf );
			for( size_t idx = 0; idx < nItemCount; ++idx )
			{
				fread( pTmpBuf, 1, nBinItemSize, pFIn );
				this->Items.push_back( new ColumnItem( (double) *pnItemData ) );
			}
		}
		break;
	case COL_TYPE_DOUBLE:
		{
			double *pdItemData;
			pdItemData = (double*)( pTmpBuf );
			nItemCount	= (unsigned int)nFileSize / nBinItemSize;
			for( size_t idx = 0; idx < nItemCount; ++idx )
			{
				fread( pTmpBuf, 1, nBinItemSize, pFIn );
				this->Items.push_back( new ColumnItem( *pdItemData ) );
			}
		}
		break;
	case COL_TYPE_VARCHAR:
		do
		{
			size_t idx2 = 0;
			do 
			{
				fread( &ch, 1, sizeof(char), pFIn );
				if( feof(pFIn) )
					break;
				pTmpBuf[idx2++] = ch;
			} while( ch != '\0' );
			if( ch != '\0' )
				pTmpBuf[idx2] = '\0';
			if( idx2 > 0 )
				this->Items.push_back( new ColumnItem((char*)pTmpBuf) );
		}
		while( !feof(pFIn) );
		break;
	default:
		assert( 0 );
	}
}

//------------------------------------------------------------------------------
void Column::saveToFile(	const std::string& file, size_t startIdx, size_t endIdx, 
							bool append )
{
	size_t totalCount = 0;
	if( this->Count )
		for( size_t idx = 0; idx < this->Count->Items.size(); ++idx )
			totalCount += (int) this->Count->Items[idx]->numval;
	else
		totalCount = this->Items.size();
	if( endIdx == std::string::npos )
		endIdx = totalCount;
	if( (startIdx < 0) || (endIdx > totalCount) )
		throw generic_error(generic_error::GENERIC, "");

	FILE	*pFIn;
	if( append )
		pFIn = fopen( file.c_str(), "ab" );
	else
		pFIn = fopen( file.c_str(), "wb" );
	FileCloser fileCloser(pFIn);

	int countLimit = 0;
	int countIdx = 0;
	if( this->Count )
	{
		while( countLimit < startIdx )
			countLimit += (int) this->Count->Items[countIdx++]->numval;
	}

	size_t realIdx;
	for(size_t idx = startIdx; idx < endIdx; ++idx )
	{
		if( this->Count )
		{
			if( idx >= countLimit )
				countLimit += (int) this->Count->Items[countIdx++]->numval;
			realIdx = countIdx - 1;
		}
		else
			realIdx = idx;
	
		switch( this->Type )
		{
		case COL_TYPE_INT:
			{
				int val = 'NULL';
				if( this->Items[realIdx] )
					val = (int) this->Items[realIdx]->numval;
				fwrite( &val, 1, sizeof(int), pFIn );
			}
			break;
		case COL_TYPE_BIG_INT:
		case COL_TYPE_DATE1:
		case COL_TYPE_DATE2:
		case COL_TYPE_DATE3:
			{
				llong val = 'NULL';
				if( this->Items[realIdx] )
					val = (llong) this->Items[realIdx]->numval;
				fwrite( &val, 1, sizeof(llong), pFIn );
			}
			break;
		case COL_TYPE_DOUBLE:
			{
				double val = 'NULL';
				if( this->Items[realIdx] )
					val = this->Items[realIdx]->numval;
				fwrite( &val, 1, sizeof(double), pFIn );
			}
			break;
		case COL_TYPE_VARCHAR:
			{
				std::string val;
				if( this->Items[realIdx] )
					fwrite( this->Items[realIdx]->strval.c_str(), sizeof(char), 
					this->Items[realIdx]->strval.length(), pFIn );
				else
					fwrite( "NULL", sizeof(char), strlen("NULL"), pFIn );
				fputc( '\0', pFIn );
			}
			break;
		default:
			assert( 0 );
		}
	}
}

//void Column::addItem(size_t index, const TProjectSettings& settings, const Base& BaseDesc)
//{
//	size_t numPack = index / ((size_t) settings.packSize);
//	this->packOffset = index % settings.packSize;
//
//	if( numPack != this->currentNumPack )
//	{
//		char szBuffer[128]; // fixme
//		string dataPath = settings.szRootPath;
//		sprintf( szBuffer, "data_orga\\vdg\\data\\B001T%.4uC%.4uV01P%.12u", this->TableID, this->ID, numPack );
//		dataPath += szBuffer;
//
//		string prmFilePath = dataPath + ".prm";
//		string theFilePath = dataPath + ".the";
//
//		aq::Logger::getInstance().log(AQ_DEBUG, "Open prm file %s\n", prmFilePath.c_str());
//		aq::Logger::getInstance().log(AQ_DEBUG, "Open thesaurus %s\n", theFilePath.c_str());
//
//		prmMapper.reset(new aq::FileMapper(prmFilePath.c_str()));
//		thesaurusMapper.reset(new aq::FileMapper(theFilePath.c_str()));
//
//		this->currentNumPack = numPack;
//	}
//
//
//	size_t prmOffset = this->packOffset * this->prmFileItemSize;
//	uint32_t theOffset;
//	prmMapper->read(&theOffset, prmOffset, prmFileItemSize);
//
//	switch( this->Type )
//	{
//	case COL_TYPE_INT:
//		{
//			int32_t *pnItemData;
//			pnItemData = (int*)( this->pTmpBuf );
//			thesaurusMapper->read(this->pTmpBuf, this->nBinItemSize * theOffset, this->nBinItemSize);
//			if( *pnItemData == 'NULL' )
//				this->Items.push_back( NULL );
//			else
//				this->Items.push_back( new ColumnItem( *pnItemData ) );
//		}
//		break;
//	case COL_TYPE_BIG_INT:
//	case COL_TYPE_DATE1:
//	case COL_TYPE_DATE2:
//	case COL_TYPE_DATE3:
//		{
//			int64_t *pnItemData;
//			pnItemData = (long long*)( this->pTmpBuf );
//			thesaurusMapper->read(this->pTmpBuf, this->nBinItemSize * theOffset, this->nBinItemSize);
//			if( *pnItemData == 'NULL' )
//				this->Items.push_back( NULL );
//			else
//				this->Items.push_back( new ColumnItem( (double) *pnItemData ) );
//		}
//		break;
//	case COL_TYPE_DOUBLE:
//		{
//			double *pdItemData;
//			pdItemData = (double*)( this->pTmpBuf );
//			thesaurusMapper->read(this->pTmpBuf, this->nBinItemSize * theOffset, this->nBinItemSize);
//			if( *pdItemData == 'NULL' )
//				this->Items.push_back( NULL );
//			else
//				this->Items.push_back( new ColumnItem( *pdItemData ) );
//		}
//		break;
//	case COL_TYPE_VARCHAR:
//		{	
//			thesaurusMapper->read(this->pTmpBuf, this->nBinItemSize * theOffset, this->nBinItemSize);
//			if( strcmp((char*)this->pTmpBuf, "NULL") == 0 )
//				this->Items.push_back( NULL );
//			else
//				this->Items.push_back( new ColumnItem( string((char*)this->pTmpBuf) ) );
//		}
//		break;
//	}
//}

//------------------------------------------------------------------------------
void Column::dumpRaw( std::ostream& os )
{
  std::string colName = this->getOriginalName();
  size_t pos = colName.find('.');
  if( pos != std::string::npos )
    colName = colName.substr( pos + 1 );
  os << colName << "\" " << this->ID << " " << this->Size << " ";
  switch( this->Type )
  {
  case COL_TYPE_INT: os << "INT"; break;
  case COL_TYPE_BIG_INT: os << "BIG_INT"; break;
  case COL_TYPE_DOUBLE: os << "DOUBLE"; break;
  case COL_TYPE_DATE1: os << "DATE1"; break;
  case COL_TYPE_DATE2: os << "DATE2"; break;
  case COL_TYPE_DATE3: os << "DATE3"; break;
  case COL_TYPE_VARCHAR: os << "VARCHAR2"; break;
  default:
    throw generic_error(generic_error::NOT_IMPLEMENED, "");
  }
  os << std::endl;
}

//------------------------------------------------------------------------------
void Column::dumpXml( std::ostream& os )
{
  std::string colName = this->getOriginalName();
  size_t pos = colName.find('.');
  if( pos != std::string::npos )
    colName = colName.substr( pos + 1 );
  os << "<Column Name=\"" << colName << "\" ID=\"" << this->ID << "\" Size=\"" << this->Size << "\" Type=\"";
  switch( this->Type )
  {
  case COL_TYPE_INT: os << "INT"; break;
  case COL_TYPE_BIG_INT: os << "BIG_INT"; break;
  case COL_TYPE_DOUBLE: os << "DOUBLE"; break;
  case COL_TYPE_DATE1: os << "DATE1"; break;
  case COL_TYPE_DATE2: os << "DATE2"; break;
  case COL_TYPE_DATE3: os << "DATE3"; break;
  case COL_TYPE_VARCHAR: os << "VARCHAR2"; break;
  default:
    throw generic_error(generic_error::NOT_IMPLEMENED, "");
  }
  os << "\"/>" << std::endl;
}

}