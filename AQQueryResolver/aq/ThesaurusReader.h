#ifndef __THESAURUS_READER_H__
#define __THESAURUS_READER_H__

#include "ColumnMapper_Intf.h"
#include <aq/Database.h>
#include <aq/Exceptions.h>
#include <boost/filesystem.hpp>

namespace aq
{
  
  template <typename T, class M>
  class ThesaurusReader : public ColumnMapper_Intf<T>
  {
  public:
    ThesaurusReader(const char * _path, size_t _tableId, size_t _columnId, size_t _size, size_t _packetSize, size_t _currentPacket = 0, bool _readNextPacket = true);
    ~ThesaurusReader();
    int loadValue(size_t index, T * item);
    int setValue(size_t index, T * value);
    int append(T * value);
  private:
    boost::shared_ptr<M> thesaurusMapper;
    const std::string path;
    const size_t tableId;
    const size_t columnId;
    const size_t size;
    size_t currentPacket;
    std::vector<size_t> sizes;
    std::pair<size_t, size_t> mappedZone;
    T * val;
    bool readNextPacket;
  };
  
  template <typename T, class M>
  ThesaurusReader<T, M>::ThesaurusReader(const char * _path, size_t _tableId, size_t _columnId, size_t _size, size_t _packetSize, size_t _currentPacket, bool _readNextPacket)
    : path(_path), tableId(_tableId), columnId(_columnId), size(_size), currentPacket(_currentPacket), readNextPacket(_readNextPacket)
  {
    std::string thesaurusFilename = aq::Database::getThesaurusFileName(path.c_str(), tableId, columnId, currentPacket);
    this->thesaurusMapper.reset(new M(thesaurusFilename.c_str()));
    sizes.push_back(this->thesaurusMapper->size() / (size * sizeof(T)));
    mappedZone = std::make_pair(0, sizes[0]);
    val = new T[this->size];
  }

  template <typename T, class M>
  ThesaurusReader<T, M>::~ThesaurusReader()
  {
    delete[] val;
  }

  template <typename T, class M>
  int ThesaurusReader<T, M>::loadValue(size_t index, T * value)
  {
    int rc = 0;
    if (((index >= mappedZone.first) && (index < mappedZone.second)))
    {
      size_t packet_index = index;
      for (size_t i = 0; (i < sizes.size()) && (packet_index >= sizes[i]); ++i) 
      {
        packet_index -= sizes[i];
      }
      rc = this->thesaurusMapper->read(value, packet_index * size * sizeof(T), size * sizeof(T));
    }
    else if ((index >= mappedZone.second) && (readNextPacket))
    {
      ++currentPacket;
      std::string thesaurusFilename = aq::Database::getThesaurusFileName(path.c_str(), tableId, columnId, currentPacket);
      boost::filesystem::path f(thesaurusFilename);
      if (boost::filesystem::exists(f))
      {
        this->thesaurusMapper.reset(new M(thesaurusFilename.c_str()));
        sizes.push_back(this->thesaurusMapper->size() / (size * sizeof(T)));
        mappedZone = std::make_pair(mappedZone.second, mappedZone.second + *sizes.rbegin());
        rc = this->loadValue(index, value);
      }
      else
      {
        rc = -1;
      }
    }
    else if (index < mappedZone.first)
    {
      // TODO : backward reading is not implemented
      assert(false);
      throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "backward reading of thesaurus is not implemented");
    }
    else
    {
      rc = -1;
    }
    return rc;
  }
  
  template <typename T, class M>
  int ThesaurusReader<T, M>::setValue(size_t, T * value)
  {
    throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "set value is not allowed in thesaurus reader");
    return 0;
  }
  
  template <typename T, class M>
  int ThesaurusReader<T, M>::append(T * value)
  {
    throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "append value is not allowed in thesaurus reader");
    return 0;
  }

}

#endif
