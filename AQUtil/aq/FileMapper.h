#ifndef __FILE_MAPPER_H__
#define __FILE_MAPPER_H__

#include <windows.h>
#include <string>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace aq
{

class FileMapper
{
public:
	FileMapper(const char * _filename);
	~FileMapper();

	int read(void * buffer, size_t offset, size_t len);
  size_t size() { return this->cbFile; }

private:
	FileMapper(const FileMapper&);
	FileMapper& operator=(const FileMapper&);

	void remap(size_t offset);

	const std::string filename;

	DWORD cbView;
  HANDLE hfile;
	HANDLE hmap;
	char const * pView;

	unsigned long long cbFile;
	unsigned long long windowOffset;

	unsigned nbRemap;
};

}

#endif