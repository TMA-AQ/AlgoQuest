#ifndef __MMAP_FILE_MAPPER_H__
#define __MMAP_FILE_MAPPER_H__

#include <string>

#error "should not be include"

namespace aq
{

class MMapFileMapper
{
public:
	MMapFileMapper(const char * _filename);
	~MMapFileMapper();

	int read(void * buffer, size_t offset, size_t len);
  size_t size() { return this->m_size; }

private:
	MMapFileMapper(const MMapFileMapper&);
	MMapFileMapper& operator=(const MMapFileMapper&);
	const std::string m_filename;
  size_t m_size;
};

}

#endif