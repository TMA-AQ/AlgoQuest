#ifndef __GENERIC_FILE_MAPPER_H__
#define __GENERIC_FILE_MAPPER_H__

#include <string>

namespace aq
{

class GenericFileMapper
{
public:
	GenericFileMapper(const char * _filename);
	~GenericFileMapper();

	int read(void * buffer, size_t offset, size_t len);
  size_t size() { return this->m_size; }

private:
	GenericFileMapper(const GenericFileMapper&);
	GenericFileMapper& operator=(const GenericFileMapper&);
	const std::string m_filename;
  size_t m_size;
  FILE * m_fd;
};

}

#endif
