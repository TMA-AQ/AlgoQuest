#ifndef __FILE_CLOSER_H__
#define __FILE_CLOSER_H__

#include <cstdio>

namespace aq {

/// \brief file closer helper
/// to enable a file to be closed relatively to his scope.
class FileCloser
{
public:
	FileCloser(FILE *& pFile): pFile(pFile){};
	~FileCloser() { if( pFile ) fclose(pFile); };
private:
  FileCloser(const FileCloser& o);
  FileCloser& operator=(const FileCloser& o);
	FILE *& pFile;
};

}

#endif