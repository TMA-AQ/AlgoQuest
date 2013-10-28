#ifndef __THESAURUS_READER_H__
#define __THESAURUS_READER_H__

#include "ColumnMapper_Intf.h"
#include <aq/Exceptions.h>
#include <boost/filesystem.hpp>

namespace aq
{
  
  template <typename T, class M>
  class ThesaurusReader : public ColumnMapper_Intf
  {
  public:
    ThesaurusReader(const char * _path, size_t _tableId, size_t _columnId, size_t _size, size_t _packetSize, size_t _currentPacket = 0, bool _readNextPacket = true);
    ~ThesaurusReader();
    int loadValue(size_t index, aq::ColumnItem& item);
    const aq::ColumnType getType() const { return aq::type_conversion<T>::type; } ;
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
    std::string thesaurusFilename = getThesaurusFileName(path.c_str(), tableId, columnId, currentPacket);
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
  int ThesaurusReader<T, M>::loadValue(size_t index, aq::ColumnItem& item)
  {
    int rc = 0;
    if (((index >= mappedZone.first) && (index < mappedZone.second)))
    {
      size_t packet_index = index;
      for (size_t i = 0; (i < sizes.size()) && (packet_index >= sizes[i]); ++i) 
      {
        packet_index -= sizes[i];
      }
      if ((rc = this->thesaurusMapper->read(val, packet_index * size * sizeof(T), size * sizeof(T))) == 0)
      {
        aq::fill_item<T>(item, val, this->size);
      }
    }
    else if ((index >= mappedZone.second) && (readNextPacket))
    {
      ++currentPacket;
      std::string thesaurusFilename = getThesaurusFileName(path.c_str(), tableId, columnId, currentPacket);
      boost::filesystem::path f(thesaurusFilename);
      if (boost::filesystem::exists(f))
      {
        this->thesaurusMapper.reset(new M(thesaurusFilename.c_str()));
        sizes.push_back(this->thesaurusMapper->size() / (size * sizeof(T)));
        mappedZone = std::make_pair(mappedZone.second, mappedZone.second + *sizes.rbegin());
        rc = this->loadValue(index, item);
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

}

#endif
