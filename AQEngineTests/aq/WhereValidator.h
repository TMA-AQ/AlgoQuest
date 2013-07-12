#ifndef __WHEREVALIDATOR_H__
# define __WHEREVALIDATOR_H__

# include <string>
# include <map>
# include <boost/function.hpp>
# include <vector>

# include "WhereIn.h"
# include "WhereComparator.h"

namespace aq
{

  class WhereValidator
  {
  public:

    enum KeyWord
    {
      K_NULL = 0,
      K_IN = 2,
      K_COMP = 4,
      K_KEYWORD = K_IN | K_COMP
    };

    //-------------------------------------------------------------------------
    // Constructor/Destructor
    //-------------------------------------------------------------------------

    WhereValidator();
    ~WhereValidator();

    //-------------------------------------------------------------------------
    // Parse and cut the query to different condition(s) for the key word WHERE
    //-------------------------------------------------------------------------

    void  parseQuery(const std::string& query);
    void  createCondition(const std::string& vacheQuery, WhereValidator::KeyWord ref);

    static AWhereCondition*  createInCondition(const std::string& vacheQuery)
    {
      return new WhereIn(vacheQuery);
    }

    static AWhereCondition*  createComparatorCondition(const std::string& vacheQuery)
    {
      return new WhereComparator(vacheQuery);
    }

    //-------------------------------------------------------------------------
    // Check if the condition(s) mate with the answer
    //-------------------------------------------------------------------------

    bool  check(const aq::AQMatrix& matrix, 
                const std::map<size_t, std::vector<boost::shared_ptr<aq::ColumnMapper_Intf> > >& mapper,
                size_t i)                       const;

    //-------------------------------------------------------------------------
    // Dump
    //-------------------------------------------------------------------------

    void  dump(std::ostream& os)                const;

    //-------------------------------------------------------------------------

  private:

    KeyWord getKeyWord(const std::string& word) const;

  private:

    //-------------------------------------------------------------------------
    // Attribute
    //-------------------------------------------------------------------------

    std::map<WhereValidator::KeyWord, boost::function<AWhereCondition* (const std::string&)> >   _creator;
    std::vector<AWhereCondition*>                                                               _condition;
  };

}
#endif // __WHEREVALIDATOR_H__