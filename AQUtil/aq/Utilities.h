#ifndef __AQ_UTILITIES_H__
#define __AQ_UTILITIES_H__

#include "Symbole.h"
#include "DBTypes.h"
#include <string>
#include <vector>

//------------------------------------------------------------------------------
typedef long long llong;

//------------------------------------------------------------------------------
#define STR_BUF_SIZE 4096

//------------------------------------------------------------------------------
extern const double EPSILON;

namespace aq
{

//------------------------------------------------------------------------------
char* LoadFile( const char *pszFN );

//------------------------------------------------------------------------------
/* Ret : 0 on success, -1 on error */
void SaveFile( const char *pszFN, const char* pszToSave );

//------------------------------------------------------------------------------
/* Ret : 0 on success, -1 on error */
int FileCopy( char* pszSrcPath, char* pszDstPath );

//------------------------------------------------------------------------------
/* Ret : 0 on success, -1 on error */
int FileRename(const char* pszSrcPath, const char* pszDstPath);

//------------------------------------------------------------------------------
int GetFiles( const char* pszSrcPath, std::vector<std::string>& files );

//------------------------------------------------------------------------------
char* ReadValidLine( FILE* pFIn, char* pszTmpBuf, int nSize, int nTrimEnd );

//------------------------------------------------------------------------------
void DeleteFolder( const char* pszPath );

//------------------------------------------------------------------------------
/* Should be used when reading UTF8 files */
FILE* fopenUTF8( const char* pszFlename, char* pszMode );

//------------------------------------------------------------------------------
/* Set errno if the string contains anything else but the number */
int StrToInt( const char* psz, llong* pnVal );
int StrToDouble( const char* psz, double* pdVal  );

//------------------------------------------------------------------------------
std::wstring string2Wstring(const std::string& s);

//------------------------------------------------------------------------------
class FileCloser
{
public:
	FileCloser(FILE *pFile): pFile(pFile){};
	~FileCloser() { if( pFile ) fclose(pFile); };
private:
	FILE *pFile;
};

//------------------------------------------------------------------------------
void strtoupr( std::string& strval );

//------------------------------------------------------------------------------
void Trim( std::string& strval );

//------------------------------------------------------------------------------
char* strtoupr( char* pszStr );

//------------------------------------------------------------------------------
void ShowError( char *message );

//------------------------------------------------------------------------------
void splitLine( char *psz, char fieldSeparator, std::vector<char*>& fields, 
				bool answerFormat );

//------------------------------------------------------------------------------
void doubleToString( char* strVal, double dVal );

//------------------------------------------------------------------------------
std::string getPrmFileName( const char* path, size_t tableIdx, size_t columnIdx, size_t partIdx );
std::string getThesaurusFileName( const char* path, size_t tableIdx, size_t columnIdx, size_t partIdx );

aq::ColumnType symbole_to_column_type(aq::symbole s);

}

#endif /* __AQ_UTILITIES_H__ */
