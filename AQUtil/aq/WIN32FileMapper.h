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

/// \brief Map File into memory to increase access
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

  /// \brief construct a WIN32FileMapper
  /// \param _filename the filename to map
  /// \param mode open mode [READ|WRITE|READ_WRITE]
  WIN32FileMapper(const char * _filename, const WIN32FileMapper::mode_t mode = WIN32FileMapper::mode_t::READ);
	~WIN32FileMapper();

  /// \brief read part of the file mapped into memory
  ///
  /// The memory part of the mapped file is copied into a allocated buffer. 
  /// If part is not mapped into memory, move the segment to the corresponding part
  /// \param buffer the content of the file to read will be copied here. The caller is responsible for the allocation/deallocation of the buffer.
  /// \param offset start of the part to copy
  /// \param len size of the part to copy
  /// \return 0 if succeed, -1 if mapped failed, -2 if offset is beyond the end of file
	int read(void * buffer, size_t offset, size_t len);
  
  /// \brief write new content into mapped file
  ///
  /// An allocated buffer is copied into a memory part of the mapped file. 
  /// If part is not mapped into memory, move the segment to the corresponding part.
  /// \param buffer the data to be copied into the mapped zone. The caller is responsible for the allocation/deallocation of the buffer.
  /// \param offset start of the part to be written
  /// \param len size of the part to be written
  /// \return 0 if succeed, -1 if mapped failed, -2 if offset is beyond the end of file
  int write(void * buffer, size_t offset, size_t len);
  
  /// \brief move part of the file mapped into memory
  ///
  /// The memory part of the mapped file is moved into another location in the file. 
  /// If part is not mapped into memory, move the segment to the corresponding part (both for old and new position).
  /// \param new_offset the new position
  /// \param old_offset the old position
  /// \param size size of the part to move
  /// \return 0 if succeed, -1 if failed
  int move(size_t new_offset, size_t old_offset, size_t size);
  
  /// \brief erase part of the file mapped into memory
  ///
  /// The memory part of the mapped file is erase. The size of the file is updated. 
  /// \param offset start of the part to erase
  /// \param len size of the part to erase
  /// \return 0 if succeed, -1 if mapped failed, -2 if offset is beyond the end of file
  int erase(size_t offset, size_t len);
  
  /// \brief get file size
  /// \return file size
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