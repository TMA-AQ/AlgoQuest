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
  enum mode_t
  {
    READ,
    WRITE,
    READ_WRITE,
  };

public:
	WIN32FileMapper(const char * _filename, const WIN32FileMapper::mode_t mode = WIN32FileMapper::mode_t::READ);
	~WIN32FileMapper();

	int read(void * buffer, size_t offset, size_t len);
  int write(void * buffer, size_t offset, size_t len);
  int move(size_t new_offset, size_t old_offset, size_t size);
  int erase(size_t offset, size_t len);
  size_t size() { return (size_t)this->cbFile; }

private:
	WIN32FileMapper(const WIN32FileMapper&);
	WIN32FileMapper& operator=(const WIN32FileMapper&);
  
  void resize(size_t len);
	void remap(unsigned long long offset);

	const std::string filename;

	DWORD cbView;
  HANDLE hfile;
	HANDLE hmap;
	char * pView;

	unsigned long long cbFile;
	unsigned long long windowOffset;
  mode_t mode;

	unsigned nbRemap;
};

}

#endif