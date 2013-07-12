#include "WhereValidator.h"

namespace aq
{

  //-------------------------------------------------------------------------------

  WhereValidator::WhereValidator()
  {
    this->_creator[K_IN] = this->createInCondition;
    this->_creator[K_COMP] = this->createComparatorCondition;
  }

  WhereValidator::~WhereValidator()
  {}

  //-------------------------------------------------------------------------------

  void  WhereValidator::parseQuery(const std::string& query)
  {
    std::string key = "WHERE";
    std::string::size_type pos = query.find(key);
    while (pos != std::string::npos)
    {
      std::string::size_type end = query.find("\n", pos);
      std::string line = query.substr(pos, end - pos);
      std::vector<std::string>  vecter;
      boost::replace_all(line, "\n", "");
      boost::replace_all(line, "WHERE", "");
      boost::trim(line);
      boost::split(vecter, line, boost::is_any_of(" "));
      for (auto& it : vecter)
      {
        if (this->getKeyWord(it) != WhereValidator::K_NULL)
          this->createCondition(line, this->getKeyWord(it));
        if (it == ";" || it == "ORDER" || it == "GROUP")
          return;
      }
      pos = end + 1;
    }
  }

  void  WhereValidator::createCondition(const std::string& cutQuery, WhereValidator::KeyWord ref)
  {
    if (ref & K_KEYWORD)
      this->_condition.push_back(this->_creator[ref](cutQuery));
    else
      exit(42); // message d'erreur!!
  }

  //-------------------------------------------------------------------------------

  WhereValidator::KeyWord WhereValidator::getKeyWord(const std::string& word) const
  {
    std::map<std::string, WhereValidator::KeyWord> mapper;

    mapper["IN"] = WhereValidator::K_IN;
    mapper["K_JEQ"] = WhereValidator::K_COMP;
    mapper["K_JIEQ"] = WhereValidator::K_COMP;
    mapper["K_JINF"] = WhereValidator::K_COMP;
    mapper["K_JSEQ"] = WhereValidator::K_COMP;
    mapper["K_JSUP"] = WhereValidator::K_COMP;

    if (mapper.find(word) != mapper.end())
      return mapper[word];
    return WhereValidator::K_NULL;
  }

  //-------------------------------------------------------------------------------

  bool  WhereValidator::check(const aq::AQMatrix& matrix, 
                              const std::map<size_t, std::vector<boost::shared_ptr<aq::ColumnMapper_Intf> > >& mapper,
                              size_t i)         const
  {
    for (auto& it : this->_condition)
      if (!it->checkCondition(matrix, mapper, i))
        return false;
    return true;
  }

  //-------------------------------------------------------------------------------

  void  WhereValidator::dump(std::ostream& os)  const
  {
    for (auto& it : this->_condition)
      it->dump(os);
  }

}