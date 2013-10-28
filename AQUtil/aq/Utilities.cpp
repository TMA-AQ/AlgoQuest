#include "Utilities.h"
#include "DateConversion.h"
#include "Exceptions.h"
#include "Logger.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <boost/filesystem.hpp>

#ifdef WIN32
#include <Windows.h>
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif

namespace aq
{

//------------------------------------------------------------------------------

extern const double EPSILON = 0.0000001;

//------------------------------------------------------------------------------
char* LoadFile( const char *pszFN ) {
	char* pszBuf = NULL;
	FILE* pFIn = NULL;
	long  nFileSize = 0;
	int   nPos = 0;

	pFIn = fopenUTF8( pszFN, "rb" );
	if ( pFIn == NULL )
		return NULL;

	/* Get start position */
	nPos = ftell( pFIn );
	/* Seek to end */
	if ( fseek( pFIn, 0, SEEK_END ) != 0 ) {
		fclose( pFIn );
		return NULL;
	}
	/* Get File Size */
	nFileSize = ftell( pFIn ) - nPos;
	if ( nFileSize <= 0 ) {
		fclose( pFIn );
		return NULL;
	}
	/* Seek to beginning */
	if ( fseek( pFIn, nPos, SEEK_SET ) != 0 ) {
		fclose( pFIn );
		return NULL;
	}
	/* Allocate Memory for the file content + '\0' character */	
	pszBuf = new char[nFileSize + 1];
	if ( pszBuf == NULL ) {
		fclose( pFIn );
		return NULL;
	}
	/* Read in the file content */
	if ( fread( pszBuf, nFileSize, 1, pFIn ) != 1 ) {
		fclose( pFIn );
		delete[] pszBuf;
		return NULL;
	}
	/* '\0' terminate the string */
	pszBuf[ nFileSize ] = '\0';

	fclose( pFIn );

	return pszBuf;
}

//------------------------------------------------------------------------------
void SaveFile( const char *pszFN, const char* pszToSave ) {
	FILE *pFOut;

	pFOut = fopen( pszFN, "wt" );
	if ( pFOut == NULL )
		throw generic_error(generic_error::GENERIC, "");
	if ( fputs( pszToSave, pFOut ) < 0 ) {
		fclose( pFOut );
		throw generic_error(generic_error::GENERIC, "");
	}
	fclose( pFOut );
}

//------------------------------------------------------------------------------
int FileCopy( char* pszSrcPath, char* pszDstPath )
{
	if( strcmp(pszSrcPath, pszDstPath) == 0 )
		return 0;

	FILE *fsrc = NULL;
	FILE *fdst = NULL;
	char c;

	fsrc = fopen( pszSrcPath, "rb" );
	if( fsrc == NULL )
		return -1;

	fdst = fopen( pszDstPath, "wb" );
	if( fdst == NULL )
		return -1;

	while( fread(&c, 1, 1, fsrc ) == 1 )
		if( fwrite(&c, 1, 1, fdst) != 1 ) 
		{
			fclose( fsrc );
			fclose( fdst );
			return -1;
		}

	fclose( fsrc );
	fclose( fdst );
	return 0;
}

//------------------------------------------------------------------------------
int FileRename(const char* pszSrcPath, const char* pszDstPath)
{
	boost::system::error_code ec;
	boost::filesystem::path oldFile(pszSrcPath); 
	boost::filesystem::path newFile(pszDstPath);
	boost::filesystem::rename(oldFile, newFile, ec);
	if (ec)
	{
		aq::Logger::getInstance().log(AQ_ERROR, "Cannot rename file %s to %s\n", pszSrcPath, pszDstPath);
		return -1;
	}
	aq::Logger::getInstance().log(AQ_DEBUG, "rename file %s to %s\n", pszSrcPath, pszDstPath);
	return 0;
}

//------------------------------------------------------------------------------
int GetFiles( const char* pszSrcPath, std::vector<std::string>& files )
{
#ifdef WIN32
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	std::string path( pszSrcPath );
	path += "\\*";
	hFind = FindFirstFile( path.c_str(), &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
		return -1;

	do
	{
		if( !(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			files.push_back( ffd.cFileName );
	}
	while (FindNextFile(hFind, &ffd) != 0);

	DWORD dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) 
		return -1;

	FindClose(hFind);
#else
	//not implemented yet
#endif
	return 0;
}

//------------------------------------------------------------------------------
char* ReadValidLine( FILE* pFIn, char* pszTmpBuf, int nSize, int nTrimEnd ) {
	char *psz = NULL;
	size_t  nLen;

	/* Read a valid line */
	pszTmpBuf[ 0 ] = '\0';
	while ( pszTmpBuf[ 0 ] == '\0' ) {
		if ( fgets( pszTmpBuf, nSize, pFIn ) == NULL )
			return NULL;

		/* Skip whitespace from the begin of the line ! */
		psz = pszTmpBuf;
		while ( *psz != '\0' && ( *psz == ' ' || *psz == '\t' ) )
			psz++;

		/* Check if valid line ! */
		if ( *psz == '\0' || *psz == '\n' || *psz == '\r' )
			pszTmpBuf[ 0 ] = '\0';
	}

	nLen = strlen( psz );
	while ( nLen > 0 ) {
		if ( psz[ nLen - 1 ] != '\n' && psz[ nLen - 1 ] != '\r'
			&& (!nTrimEnd || psz[ nLen - 1 ] != ' ') )
			break;
		nLen--;
		psz[ nLen ] = '\0';
	}

	return psz;
}

//------------------------------------------------------------------------------
void CleanFolder( const char * pszPath )
{
	boost::filesystem::path p(pszPath);
	if (!boost::filesystem::exists(p))
	{
		aq::Logger::getInstance().log(AQ_INFO, "path %s doesn't exists\n", p.string().c_str());
	}
	else
	{
		boost::system::error_code ec;
    for (boost::filesystem::directory_iterator file(p); file != boost::filesystem::directory_iterator(); ++file)
    {
      const std::string& filename = (*file).path().string();
      if ((filename.size() > 4) && (filename.substr(filename.size() - 4) == ".TMP")) continue; // FIXME
      if (!boost::filesystem::remove_all(*file, ec))
      {
        aq::Logger::getInstance().log(AQ_ERROR, "cannot delete path %s\n", (*file).path().string().c_str());
      }
      else
      {
        aq::Logger::getInstance().log(AQ_DEBUG, "delete path %s\n", (*file).path().string().c_str());
      }
    }
	}
}

//------------------------------------------------------------------------------
void DeleteFolder( const char * pszPath )
{
	boost::filesystem::path p(pszPath);
	if (!boost::filesystem::exists(p))
	{
		aq::Logger::getInstance().log(AQ_INFO, "path %s doesn't exists\n", p.string().c_str());
	}
	else
	{
		boost::system::error_code ec;
		if (!boost::filesystem::remove_all(p, ec))
		{
			aq::Logger::getInstance().log(AQ_ERROR, "cannot delete path %s\n", p.string().c_str());
		}
		else
		{
			aq::Logger::getInstance().log(AQ_DEBUG, "delete path %s\n", p.string().c_str());
		}
	}
}

//------------------------------------------------------------------------------
FILE* fopenUTF8( const char* pszFlename, const char* pszMode )
{
	FILE	*fp = NULL;
	unsigned char bom[3];
	fp = fopen(pszFlename, pszMode);
	if( fp == NULL )
		return NULL;

	/* Skip UTF-8 BOM if present */
	if( fread( bom, 3, sizeof(char), fp ) != 1 )
	{
		fclose( fp );
		return NULL;
	}
	if( !((bom[0] == 0xEF) && (bom[1] == 0xBB) && (bom[2] == 0xBF)) )
	{
		/* BOM not present, return to the beginning */
		if ( fseek( fp, 0, SEEK_SET ) != 0 ) {
			fclose( fp );
			return NULL;
		}
	}
	return fp;
}

//------------------------------------------------------------------------------
#if defined(_MSC_VER)
#define strtoll _strtoi64
#endif
int StrToInt( const char* psz, llong* pnVal )
{
	char* pszIdx;
	if( pnVal == NULL || psz == NULL )
		return -1;
	if( psz[0] <= ' ' )
		return -1;
	errno = 0;
	*pnVal = strtoll(psz, &pszIdx, 10);
	if( errno != 0 )
		return -1;
	if( *pszIdx != '\0' )
		return -1;
	return 0;
}

int StrToDouble( const char* psz, double* pdVal  )
{
	char* pszIdx;
	if( pdVal == NULL || psz == NULL )
		return -1;
	if( psz[0] <= ' ' )
		return -1;
	errno = 0;
	*pdVal = strtod(psz, &pszIdx);
	if( errno != 0 )
		return -1;
	if( *pszIdx != '\0' )
		return -1;
	return 0;
}

//------------------------------------------------------------------------------
char* strtoupr( char* pszStr ) {
	char *psz;

	if ( pszStr == NULL )
		return NULL;

	psz = pszStr;
	while ( *psz != '\0' )
	{
		*psz = toupper( *psz );
		psz += 1;
	}
	return pszStr;
}

//------------------------------------------------------------------------------
std::wstring string2Wstring(const std::string& s)
{
#ifdef WIN32
  int len;
  int slength = (int)s.length() + 1;
  len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
  wchar_t* buf = new wchar_t[len];
  MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
  std::wstring r(buf);
  delete[] buf;
  return r;
#else
  assert(false);
  (void)s;
#endif
}

//------------------------------------------------------------------------------
void ShowError( char *message ) {
	if ( message && *message )
		fprintf( stderr, "%s", message );
	if ( errno != 0 ) {
		fprintf( stderr, " ( " );
		perror( NULL );
		fprintf( stderr, " )\n" );
	} else
		fprintf( stderr, "\n" );
}

//------------------------------------------------------------------------------
//helper function for loadFromAnswer
void splitLine( char *psz, char fieldSeparator, std::vector<char*>& fields, 
				bool answerFormat )
{
	char *pszIdx = psz;
	if( answerFormat )
		++psz;
	while( true )
	{
		++pszIdx;
		if( *pszIdx != fieldSeparator && *pszIdx != '\0' )
			continue;
		//skip space character that engine adds to the beginning of fields
		if( answerFormat )
		{
			while( *psz == ' ' ) ++psz;
			if( *pszIdx == fieldSeparator )
			{
				char *pszEnd = pszIdx - 1;
				while( *pszEnd == ' ' && pszEnd > pszIdx ) --pszEnd;
			}
		}
		fields.push_back( psz );
		if( *pszIdx == '\0' )
			break; //we have reached the end
		//replace separator with null character to get substring
		*pszIdx = '\0';
		psz = pszIdx + 1;
	}
}

//------------------------------------------------------------------------------
void doubleToString( char* strVal, double dVal )
{
	llong iVal = (llong)(dVal * 100 + ((dVal > 0.0) ? 0.5 : -0.5));
	dVal = (double) iVal / 100;
	sprintf( strVal, "%.2lf", dVal );
}

//------------------------------------------------------------------------------
std::string getPrmFileName( const char* path, size_t tableIdx, size_t columnIdx, size_t partIdx )
{
	char szFN[ _MAX_PATH ];
	if( path )
		sprintf( szFN, "%sB001T%.4uC%.4uV01P%.12u.prm", path, tableIdx, columnIdx, partIdx );
	else
		sprintf( szFN, "B001T%.4uC%.4uV01P%.12u.prm", tableIdx, columnIdx, partIdx );
	return szFN;
}

//------------------------------------------------------------------------------
std::string getThesaurusFileName( const char* path, size_t tableIdx, size_t columnIdx, size_t partIdx )
{
	char szFN[ _MAX_PATH ];
	if( path )
		sprintf( szFN, "%sB001T%.4uC%.4uV01P%.12u.the", path, tableIdx, columnIdx, partIdx );
	else
		sprintf( szFN, "B001T%.4uC%.4uV01P%.12u.the", tableIdx, columnIdx, partIdx );
	return szFN;
}

//------------------------------------------------------------------------------
std::string getTemporaryFileName( size_t tableIdx, size_t columnIdx, size_t partIdx, const char * type, size_t size )
{
	char szFN[ _MAX_PATH ];
  sprintf( szFN, "B001TMP%.4uC%.4u%s%.4uP%.12u.TMP", tableIdx, columnIdx, type, size, partIdx );
	return szFN;
}

//------------------------------------------------------------------------------
void getFileNames( const char* path, std::vector<std::string>& filenames, const char * prefix )
{
  boost::filesystem::path p(path);
	if (!boost::filesystem::exists(p))
	{
		aq::Logger::getInstance().log(AQ_INFO, "path %s doesn't exists\n", p.string().c_str());
	}
	else
	{
		boost::system::error_code ec;
    for (boost::filesystem::directory_iterator file(p); file != boost::filesystem::directory_iterator(); ++file)
    {
      if ((prefix != NULL) && ((*file).path().string().find(prefix) != std::string::npos))
      {
        filenames.push_back((*file).path().string());
      }
    }
	}
}

//------------------------------------------------------------------------------
aq::ColumnType symbole_to_column_type(symbole s)
{
  switch (s)
  {
  case t_int: return aq::COL_TYPE_INT; break;
  case t_long_long: return aq::COL_TYPE_BIG_INT; break;
  case t_date1: return aq::COL_TYPE_DATE; break;
  case t_date2: return aq::COL_TYPE_DATE; break;
  case t_date3: return aq::COL_TYPE_DATE; break;
  case t_char: return aq::COL_TYPE_VARCHAR; break;
  case t_double: return aq::COL_TYPE_DOUBLE; break;
  case t_raw: return aq::COL_TYPE_VARCHAR; break;
  default: return aq::COL_TYPE_VARCHAR;
  }
}

//-------------------------------------------------------------------------------
void CleanSpaceAtEnd ( char *my_field )
{
	// discard all space at the end
	size_t max_size = strlen( my_field);
	// beware >0, must have at least one char
	for ( size_t i = max_size -1; i > 0 ; i -- )
	{
		if ( my_field [ i ] == ' ' ) my_field [ i ] = '\0';
		else return;
	}
	//at this point  my_field is empty 
	//need this ;   strcpy ( my_field, "NULL" ); ?
}
//-------------------------------------------------------------------------------
void ChangeCommaToDot (  char *string )
{
	// assume  : input is to be converted in double
	// change  ',' in '.'
	char *p;
	// seach first  ',' in string
	p = strchr(string, ',' );
	// modify string ',' become '.'  
	if (p != NULL )  *p = '.' ;
}

//------------------------------------------------------------------------------
int MakeBackupFile( const std::string& pszPath, backup_type_t type, int level, int id )
{
	char szBuffer[STR_BUF_SIZE];
	memset(szBuffer, 0, STR_BUF_SIZE);
	char szDstPath[_MAX_PATH];
	size_t	len = 0;
	strcpy( szBuffer, pszPath.c_str() );
	len = strlen(szBuffer);
	if( len < 3 )
	{
		aq::Logger::getInstance().log(AQ_DEBUG, "MakeBackupFile : Invalid filename %s !", szBuffer );
		return -1;
	}
	szBuffer[len - 4] = '\0';
	std::string typeChar = "";
	switch( type )
	{
  case backup_type_t::Empty: break;
  case backup_type_t::Before: typeChar = "_Before"; break;
	case backup_type_t::After: typeChar = "_After"; break;
	case backup_type_t::Exterior_Before: typeChar = "_Exterior_Before"; break;
	case backup_type_t::Exterior: typeChar = "_Exterior"; break;
	default: ;
	}
	sprintf( szDstPath, "%s_%.2d_%.2d%s.%s", szBuffer, level, id, typeChar.c_str(), &pszPath[len - 3] );
	if( FileRename( pszPath.c_str(), szDstPath ) != 0 )
	{
	  aq::Logger::getInstance().log(AQ_DEBUG, "MakeBackupFile : Error renaming file %s to %s !\n", pszPath.c_str(), szDstPath );
		return -1;
	}
	return 0;
}

//-------------------------------------------------------------------------------
void FileWriteEnreg( aq::ColumnType col_type, const int col_size, char *my_field, FILE *fcol )
{
	int dum_int;
	int * my_int = & dum_int;

	double dum_double; // 2009/09/01 
	double * my_double = &dum_double; // 2009/09/01 

	long long int dum_long_long;
	long long int *my_long_long = &dum_long_long;

  DateConversion dateConverter;

	if ( (int) strlen ( my_field ) >= col_size ) my_field[ col_size ] = 0 ;

	switch (  col_type )
	{
	case COL_TYPE_INT :
		if ( strcmp ( my_field, "NULL")  ==   0 )  *my_int = 'NULL'; // FIXME
		else  *my_int = atoi ( my_field );
		fwrite( my_int , sizeof(int), 1, fcol  );
		break;

	case COL_TYPE_BIG_INT :
		if ( strcmp ( my_field, "NULL")  ==   0 )  *my_long_long  = 'NULL'; // FIXME
#ifdef WIN32
		else  *my_long_long  = _atoi64 (my_field );   
#else
		else  *my_long_long  = atoll (my_field );   
#endif
		fwrite( my_long_long , sizeof(long long), 1, fcol );
		break;

	case COL_TYPE_DOUBLE :
		if (  strcmp ( my_field, "NULL")  ==   0 )  *my_double = 'NULL'; // FIXME
		else
		{
			// step 1 convert ',' in '.'
			ChangeCommaToDot (  my_field );
			// step 2 : use strtod
			*my_double =     strtod ( my_field, NULL );  // atof  ( field );
		}
		fwrite( my_double, sizeof(double), 1, fcol );
		break;

	case COL_TYPE_DATE:
		{
			if ( (strcmp ( my_field, "NULL")  ==   0) ||	(strcmp( my_field, "" ) == 0) )
      {
        *my_long_long  = 'NULL'; // FIXME
      }
      else
			{
				dateConverter.dateToBigInt(my_field);
        fwrite( my_long_long , sizeof(long long), 1, fcol );
			}
		}
		break;

	case COL_TYPE_VARCHAR :
		// check my_field size
		if ( (int) strlen ( my_field ) >= col_size ) my_field[ col_size ] = 0 ;
		// clean all space at the end
		CleanSpaceAtEnd (my_field );
		// write string record and go to next
		fwrite(my_field, sizeof(char), strlen( my_field ) , fcol );
		fwrite("\0",sizeof(char),1, fcol );
		break;

	default:
		throw generic_error(generic_error::TYPE_MISMATCH, "");
		break;
	}
}

}
