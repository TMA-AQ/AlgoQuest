#include <stdio.h>
#include <stdarg.h>
#include "Log.h"

#define LOG_FILE_NAME	"Log.txt"

//------------------------------------------------------------------------------
void Log( const char* pszFmt, ... ) {
	va_list pArgs;
	FILE *pFile;

	va_start( pArgs, pszFmt );
	pFile = fopen( LOG_FILE_NAME, "at+" );
	if ( pFile != NULL ) {
		vfprintf( pFile, pszFmt, pArgs );
		fclose( pFile );
	}
	va_end( pArgs );
}
