#ifndef __FIAN_UTILITIES_H__
#define __FIAN_UTILITIES_H__

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

//------------------------------------------------------------------------------
typedef long long llong;
struct tnode;

//------------------------------------------------------------------------------
#define STR_BUF_SIZE 4096

//------------------------------------------------------------------------------
extern const double EPSILON;

//------------------------------------------------------------------------------
struct TProjectSettings {
  typedef boost::shared_ptr<TProjectSettings> Ptr;

	std::string iniFile;
	std::string output;
	std::string queryIdent;
  std::string szRootPath;
	std::string szEnginePath;
	
	std::string szTempRootPath;
	std::string szCutInColPath;
	std::string szLoaderPath;
	char szSQLReqFN[ _MAX_PATH ];
	char szDBDescFN[ _MAX_PATH ];
	char szOutputFN[ _MAX_PATH ];
	char szAnswerFN[ _MAX_PATH ];
	char szThesaurusPath[ _MAX_PATH ];
	char szTempPath1[ _MAX_PATH ];
	char szTempPath2[ _MAX_PATH ];
	char szEngineParamsDisplay[ STR_BUF_SIZE ];
	char szEngineParamsNoDisplay[ STR_BUF_SIZE ];
	char fieldSeparator;
	static const int MAX_COLUMN_NAME_SIZE = 255;
	boost::optional<unsigned int> worker;
	int packSize;
	int maxRecordSize;
	bool computeAnswer;
	bool csvFormat;
	bool executeNestedQuery;
	bool useRowResolver;

	TProjectSettings();
	TProjectSettings(const TProjectSettings&);
	~TProjectSettings();
	TProjectSettings& operator=(const TProjectSettings&);

  void load(const std::string& iniFile, const std::string& queryIdent);
	void load(const std::string& iniFile);
	void changeIdent(const std::string& queryIdent);
  void dump(std::ostream& os) const;
  void writeAQEngineIni(std::ostream& os) const;
};

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
void getColumnsList( tnode* pNode, std::vector<tnode*>& columns );

//------------------------------------------------------------------------------
void doubleToString( char* strVal, double dVal );

//------------------------------------------------------------------------------
std::string getThesaurusFileName( char* path, int tableIdx, int columnIdx, int partIdx );

#endif /* __FIAN_UTILITIES_H__ */
