#include "FileMapper.h"
#include "Logger.h"
#include "Exceptions.h"
#include <iostream>
#include <stdint.h>

using namespace aq;

FileMapper::FileMapper(const char * _filename)
	: m_filename(_filename)
{
  this->m_fd = fopen(_filename, "rb");
  if (this->m_fd != NULL)
  {
    fseek(this->m_fd, 0, SEEK_END);
    this->m_size = ftell(this->m_fd);
    fseek(this->m_fd, 0, SEEK_SET);
  }
}

FileMapper::~FileMapper()
{
  fclose(this->m_fd);
}

int FileMapper::read(void * buffer, size_t offset, size_t len) 
{
  assert(this->m_fd);
  assert(offset < std::numeric_limits<size_t>::max());
  if (this->m_fd == NULL)
  {
    throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, m_filename);
  }
  if (offset >= std::numeric_limits<size_t>::max())
  {
    throw aq::generic_error(aq::generic_error::GENERIC, "bad offset of mapped file " + m_filename);
  }
  
  fseek(this->m_fd, static_cast<long>(offset), SEEK_SET);
  fread(buffer, 1, len, this->m_fd);
  if (feof(this->m_fd))
    return -1;
  return 0;
}

