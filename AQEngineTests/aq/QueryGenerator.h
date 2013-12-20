#ifndef __AQ_QUERY_GENERATOR_H__
#define __AQ_QUERY_GENERATOR_H__

#include <sstream>
#include <vector>
#include <map>
#include <cassert>

namespace aq
{

  class QueryGenerator
  {
  public:
    typedef std::vector<std::string> idents_t;
    typedef std::map<std::string, idents_t> ops_t;
    static void parse(std::istream& is, std::string& base, ops_t& _ops, idents_t& _idents);
  public:
    QueryGenerator(std::istream& is);
    QueryGenerator(const std::string& base, const ops_t& ops, const idents_t& idents);
    ~QueryGenerator();
    void reset();
    std::string next();
    size_t getNbQueries() const;
  private:
    void initIdents(const idents_t& idents);
    void resetValue(ops_t::value_type& value);
    QueryGenerator(const QueryGenerator& o);
    QueryGenerator& operator=(const QueryGenerator& o);
    std::string base;
    ops_t ops;
    ops_t values;
  };

}

#endif