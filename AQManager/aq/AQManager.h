#if defined(WIN32)
# ifdef AQLIB_EXPORTS
#  define AQLIB_API __declspec(dllexport)
# else
#  define AQLIB_API __declspec(dllimport)
# endif
  
# define AQLIB_API __stdcall

#endif

#include <string>

extern "C"
{
AQLIB_API int solve_query(const char * query, const char * iniFilename, const char * workingDirectory, 
                          const char * logIdent, const char * logMode, unsigned int logLevel,
                          bool clean, bool force);

AQLIB_API int load_db(const char * propertiesFile, unsigned int tableId);

AQLIB_API const char * aql2sql(const char * aql_query);
}
