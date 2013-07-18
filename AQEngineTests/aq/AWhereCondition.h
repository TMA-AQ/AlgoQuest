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

  class AWhereCondition
  {
  public:
    AWhereCondition(const std::string& query) : _query(query)
    {
    }

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

    virtual void        setValues(const aq::Base& baseDesc) = 0;

  protected:
    std::string   _query;
  };

}

#endif // AWHERECONDITION_H__