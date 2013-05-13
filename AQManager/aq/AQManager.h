#ifdef AQLIB_EXPORTS
#define AQLIB_API __declspec(dllexport)
#else
#define AQLIB_API __declspec(dllimport)
#endif

#define CALLBACK __stdcall

#include <string>

extern "C"
{
AQLIB_API int solve_query(const char * query, const char * iniFilename, const char * workingDirectory, 
                          const char * logIdent, const char * logMode, unsigned int logLevel,
                          bool clean, bool force);

AQLIB_API int load_db(const char * propertiesFile, unsigned int tableId);

AQLIB_API int test_aq_lib(const char * query);
}