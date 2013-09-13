#include "FileMapper.h"
#include "Logger.h"
#include <iostream>
#include <cstdint>

using namespace aq;

FileMapper::FileMapper(const char * _filename)
	: m_filename(_filename)
{
  this->m_fd = fopen(_filename, "rb");
}

FileMapper::~FileMapper()
{
  fclose(this->m_fd);
}

int FileMapper::read(void * buffer, size_t offset, size_t len) 
{
  assert(this->m_fd);
  assert(offset < std::numeric_limits<size_t>::max());
  fseek(this->m_fd, static_cast<long>(offset), SEEK_SET);
  fread(buffer, 1, len, this->m_fd);
  if (feof(this->m_fd))
    return -1;
  return 0;
}

