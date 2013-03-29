#include "FileMapper.h"
#include "Logger.h"
#include <iostream>
#include <cstdint>

using namespace aq;

FileMapper::FileMapper(const char * _filename)
	: filename(_filename),
		pView(NULL),
		hmap(NULL),
		hfile(NULL),
		nbRemap(0)
{
  // Offsets must be a multiple of the system's allocation granularity.  We
  // guarantee this by making our view size equal to the allocation granularity.
	unsigned nb =  0;
  SYSTEM_INFO sysinfo = {0};
  ::GetSystemInfo(&sysinfo);
  this->cbView = sysinfo.dwAllocationGranularity * 100;
	this->windowOffset = 0;

  this->hfile = ::CreateFile(_filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	
  if (hfile != INVALID_HANDLE_VALUE)
	{
    LARGE_INTEGER file_size = {0};
    ::GetFileSizeEx(hfile, &file_size);
    this->cbFile = static_cast<unsigned long long>(file_size.QuadPart);
    this->hmap = ::CreateFileMappingW(hfile, NULL, PAGE_READONLY, 0, 0, NULL);
	}
	else 
	{
		// TODO
	}
}

FileMapper::~FileMapper()
{
	aq::Logger::getInstance().log(AQ_DEBUG, "%s: %u remaping\n", this->filename.c_str(), this->nbRemap);
	if (this->pView != NULL)
		::UnmapViewOfFile(this->pView);
	if (hmap)
		::CloseHandle(hmap);
	if (hfile)
		::CloseHandle(hfile);
}

int FileMapper::read(void * buffer, size_t offset, size_t len) 
{
	uint8_t * buf = static_cast<uint8_t*>(buffer);

	if (hmap == NULL) 
	{
		return -1;
	}
	
	// Check if offset is in the current window
	if ((pView == NULL ) || (offset < windowOffset) || ((offset + len) > (windowOffset + cbView)))
	{
		unsigned long long off_tmp = offset - (offset % cbView);
		this->remap(off_tmp);
	}
	
	size_t new_offset = offset % cbView;

	if (this->pView == NULL) 
	{
		// TODO
		exit(-1);
		return -2;
	}
	
	if ((new_offset + len) > cbView)
	{
		memcpy(buf, this->pView + new_offset, cbView - new_offset);
		offset += cbView - new_offset;
		buf += cbView - new_offset;
		len -= cbView - new_offset;
		this->remap(offset);
		new_offset = 0;
	}
	
	if (this->pView == NULL) 
	{
		// TODO
		exit(-1);
		return -2;
	}
	
	memcpy(buf, this->pView + new_offset, len);
	return 0;
}

void FileMapper::remap(size_t offset)
{
	this->nbRemap++;
	DWORD high = static_cast<DWORD>((offset >> 32) & 0xFFFFFFFFul);
	DWORD low  = static_cast<DWORD>( offset        & 0xFFFFFFFFul);
	// The last view may be shorter.
	int new_cbView = cbView;
	if (offset + cbView > cbFile) {
		new_cbView = static_cast<int>(cbFile - offset);
	}
	if (this->pView != NULL)
	{
		::UnmapViewOfFile(this->pView);
	}
	this->pView = static_cast<char const *>(::MapViewOfFile(hmap, FILE_MAP_READ, high, low, new_cbView));
	this->windowOffset = offset;
}