#ifndef __FIAN_UTILITIES_H__
#define __FIAN_UTILITIES_H__

#include <string>
#include <vector>

//------------------------------------------------------------------------------
typedef long long llong;
struct tnode;

//------------------------------------------------------------------------------
#define STR_BUF_SIZE 4096

//------------------------------------------------------------------------------
extern const double EPSILON;

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
std::string getThesaurusFileName( char* path, int tableIdx, int columnIdx, int partIdx );

#endif /* __FIAN_UTILITIES_H__ */
