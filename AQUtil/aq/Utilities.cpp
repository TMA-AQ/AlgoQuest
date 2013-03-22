#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "Utilities.h"
#include "Exceptions.h"
#include "SQLParser.h"
#include "sql92_grm_tab.h"
#include <aq/Logger.h>
#include <boost/filesystem.hpp>

#include <Windows.h>

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
	int  nLen;

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
FILE* fopenUTF8( const char* pszFlename, char* pszMode )
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
void strtoupr( std::string& strval ) {
	for( size_t idx = 0; idx < strval.length(); ++idx )
		strval[idx] = toupper( strval[idx] );
}

//------------------------------------------------------------------------------
void Trim( std::string& strval )
{
	if( strval == "" )
		return;
	int start = 0, end = (int) strval.length() - 1;
	while( (strval[start] <= 32) && (start <= end) ) ++start;
	while( (strval[end]) <= 32 && (start <= end) ) --end;
	if( (start == 0) && (end == (int) strval.length() - 1) )
		return;
	strval = strval.substr( start, end-start+1 );
}

//------------------------------------------------------------------------------
char* strtoupr( char* pszStr ) {
	char *psz;

	if ( pszStr == NULL )
		return NULL;

	psz = pszStr;
	while ( *psz != '\0' )
		*psz++ = toupper( *psz );
	return pszStr;
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
std::string getThesaurusFileName( char* path, int tableIdx, int columnIdx, int partIdx )
{
	char szFN[ _MAX_PATH ];
	if( path )
		sprintf( szFN, "%sB001T%.4uC%.4uV01P%.12u.the", path, tableIdx, columnIdx, partIdx );
	else
		sprintf( szFN, "B001T%.4uC%.4uV01P%.12u.the", tableIdx, columnIdx, partIdx );
	return szFN;
}
