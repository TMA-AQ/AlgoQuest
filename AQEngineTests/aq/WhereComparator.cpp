#include "WhereComparator.h"

namespace aq
{
  WhereComparator::WhereComparator(const std::string& query) : AWhereCondition(query)
  {
    std::map<std::string, boost::function<bool (const aq::AQMatrix& matrix, 
                                                const std::map<size_t, std::vector<boost::shared_ptr<aq::ColumnMapper_Intf> > >& mapper,
                                                size_t i)> > mapper;

    mapper["K_JEQ"] = this->checkJEQ;
    mapper["K_JIEQ"] = this->checkJIEQ;
    mapper["K_JINF"] = this->checkJINF;
    mapper["K_JSEQ"] = this->checkJSEQ;
    mapper["K_JSUP"] = this->checkJSUP;

    std::string::size_type pos;

    for (auto& it : mapper)
      if ((pos = query.find(it.first)) != std::string::npos)
      {
        this->_comparator = it.second;
        break;
      }
  }

  bool  WhereComparator::checkCondition(const aq::AQMatrix& matrix, 
                                        const std::map<size_t, std::vector<boost::shared_ptr<aq::ColumnMapper_Intf> > >& mapper,
                                        size_t i) const
  {
    return this->_comparator(matrix, mapper, i);
  }
}