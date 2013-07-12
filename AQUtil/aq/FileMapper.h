#ifndef __FILE_MAPPER_H__
#define __FILE_MAPPER_H__

#include <string>

namespace aq
{

class FileMapper
{
public:
	FileMapper(const char * _filename);
	~FileMapper();

	int read(void * buffer, size_t offset, size_t len);
  size_t size() { return this->m_size; }

private:
	FileMapper(const FileMapper&);
	FileMapper& operator=(const FileMapper&);
	const std::string m_filename;
  size_t m_size;
  FILE * m_fd;
};

}

#endif
