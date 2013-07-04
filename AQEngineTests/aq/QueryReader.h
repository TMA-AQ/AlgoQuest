#ifndef __QUERY_READER_H__
#define __QUERY_READER_H__

#include <string>
#include <sstream>

namespace aq
{

class QueryReader
{
public:
  QueryReader(std::istream& stream) : queriesStream(stream), n(1)
  {
  }

  const char * getSuite() const { return this->suite.c_str(); }
  const char * getIdent() const { return this->ident.c_str(); }
  const char * getExpected() const { return this->expected.c_str(); }
  std::string getFullIdent() const { return this->suite + "/" + this->ident ; }

  std::string next();
  bool eos() { return this->queriesStream.eof(); }

  static void extract(const std::string& line, const std::string& key, std::string& result);
  template <typename T> T extract_value(const std::string& key, const T default) const;

private:
  std::istream& queriesStream;
  std::string suite;
  std::string ident;
  std::string expected;
  unsigned int n;
};

template <typename T> 
T QueryReader::extract_value(const std::string& key, const T default) const
{
  T value = default;
  std::string::size_type b = this->expected.find(key + ":");
  if (b != std::string::npos)
  {
    std::string::size_type e = this->expected.find(";", b);
    if (e != std::string::npos)
    {
      value = boost::lexical_cast<T>(this->expected.substr(b + key.size() + 1, e - (b + key.size() + 1)));
    }
    else
    {
      value = boost::lexical_cast<T>(this->expected.substr(b + key.size() + 1));
    }
  }
  return value;
}

}

#endif