#ifndef __WINDOW_FILE_MAPPER_H__
#define __WINDOW_FILE_MAPPER_H__

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
class WIN32FileMapper
{
public:
	WIN32FileMapper(const char * _filename);
	~WIN32FileMapper();

	int read(void * buffer, size_t offset, size_t len);
  size_t size() { return this->cbFile; }

private:
	WIN32FileMapper(const WIN32FileMapper&);
	WIN32FileMapper& operator=(const WIN32FileMapper&);

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