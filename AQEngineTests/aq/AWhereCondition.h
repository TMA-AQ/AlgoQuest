#ifndef __AWHERECONDITION_H__
# define __AWHERECONDITION_H__

# include <string>
# include <map>
# include <vector>
# include <boost/algorithm/string.hpp>

# include <aq/AQMatrix.h>

namespace aq
{

  class AWhereCondition
  {
  public:
    AWhereCondition(const std::string& query) : _query(query)
    {
      std::vector<std::string>  vecter;
      boost::split(vecter, query, boost::is_any_of(" "));
      for (auto& it = vecter.begin(); it != vecter.end(); ++it)
      {
        if ((*it) == ".")
        {
          ++it;
          if (it == vecter.end())
            exit(61);
          this->_arg.push_back((*it));
          ++it;
          if (it == vecter.end())
            exit(61);
          this->_arg.push_back((*it));
        }
      }
      if (this->_arg.size() % 2 != 0)
        exit(61);
    }

    virtual ~AWhereCondition() {}

    virtual bool        checkCondition(const aq::AQMatrix& matrix, 
                                       const std::map<size_t, std::vector<boost::shared_ptr<aq::ColumnMapper_Intf> > >& mapper,
                                       size_t i)  const = 0;

    const std::string&  getQuery()                const
    {
      return _query;
    }

    void                dump(std::ostream& os)    const
    {
      os << "Condition for query: " << this->getQuery() << std::endl;
      for (auto& it = this->_arg.begin(); it != this->_arg.end(); ++it)
      {
        os << "   with table: " << (*it) << " . ";
        ++it;
        os << (*it) << std::endl;
      }
    }

  private:
    std::string               _query;
    std::vector<std::string>  _arg;
  };

}

#endif // AWHERECONDITION_H__