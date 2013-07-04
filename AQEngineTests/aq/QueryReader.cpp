#include "QueryReader.h"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

using namespace aq;

std::string QueryReader::next()
{
  // reset ident
  ident = "";
  expected = "";

  //
  // load sql queries
  std::string line;
  std::string query;
  bool fullQuery = false;
  while (!fullQuery && std::getline(this->queriesStream, line))
  {
    boost::algorithm::trim(line);
    std::string::size_type pos = line.find("--");
    if (pos != std::string::npos)
    {
      QueryReader::extract(line, "ident", ident);
      QueryReader::extract(line, "suite", suite);
      QueryReader::extract(line, "expected", expected);
      line.erase(pos);
    }
    if ((line == ""))
      continue;
    pos = line.find(";");
    if (pos != std::string::npos)
    {
      query += " " + line.substr(0, pos) + "\n";
      fullQuery = true;
    }
    else
    {
      query += " " + line + "\n";
    }
  }

  if (ident == "")
  {
    std::ostringstream identSS;
    identSS << "query " << n;
    ident = identSS.str();
  }

  n++;
  return query;
}

void QueryReader::extract(const std::string& line, const std::string& key, std::string& result)
{
  std::string::size_type b = line.find("<" + key + ">");
  std::string::size_type e = line.find("</" + key + ">");
  if ((b != std::string::npos) && (e != std::string::npos) && (b < e))
  {
    result = line.substr(b + key.size() + 2, e - (b + key.size() + 2));
  }
}
