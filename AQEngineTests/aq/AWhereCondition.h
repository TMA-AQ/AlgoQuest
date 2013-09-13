#ifndef __AWHERECONDITION_H__
# define __AWHERECONDITION_H__

# include <string>
# include <map>
# include <vector>
# include <boost/algorithm/string.hpp>

# include <aq/AQMatrix.h>
# include <aq/Base.h>

namespace aq
{

  
  typedef std::map<size_t, std::map<size_t, boost::shared_ptr<aq::ColumnMapper_Intf> > >  mapMap;
  typedef std::pair<size_t, size_t>                                                       pairSize;
  typedef std::pair<std::string, std::string>                                             pairString;

  enum   operateur
  {
    O_ERR,
    O_PLUS,
    O_MOINS,
    O_FOIS,
    O_DIV,
    O_MOD
  };

  class AWhereCondition
  {
  public:
    AWhereCondition(const std::string& query) : _query(query) {}

    virtual ~AWhereCondition() {}

    virtual bool        checkCondition(const aq::AQMatrix& matrix, mapMap& mapper, size_t i)  const = 0;

    const std::string&  getQuery()                                                            const
    {
      return _query;
    }

    void                dump(std::ostream& os)                                                const
    {
      os << "Condition for query: " << this->getQuery() << std::endl;
    }

    operateur         castOperateur(const std::string& ope)                                 const
    {
      std::map<std::string, operateur>  mapper;
      mapper["+"] = O_PLUS;
      mapper["-"] = O_MOINS;
      mapper["/"] = O_DIV;
      mapper["*"] = O_FOIS;
      mapper["%"] = O_MOD;
      if (mapper.find(ope) != mapper.end())
        return mapper[ope];
      return O_ERR;
    }

    pairSize            makePairSize(const aq::Base& baseDesc, const pairString& tabVal)
    {
      for (auto it = baseDesc.getTables().begin(); it != baseDesc.getTables().end(); ++it)
        if ((*it)->getName() == tabVal.first)
          for (auto it2 = (*it)->Columns.begin(); it2 != (*it)->Columns.end(); ++it2)
            if ((*it2)->getName() == tabVal.second)
              return std::make_pair((*it)->ID, (*it2)->ID);
      return std::make_pair(0, 0);
    }

    virtual void        setValues(const aq::Base& baseDesc) = 0;

  protected:
    std::string   _query;
  };

}

#endif // AWHERECONDITION_H__
