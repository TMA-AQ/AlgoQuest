#ifndef __WHERECOMPARATOR_H__
# define __WHERECOMPARATOR_H__

# include <boost/function.hpp>

# include "AWhereCondition.h"

namespace aq
{

  class WhereComparator : public AWhereCondition
  {
  public:
    WhereComparator(const std::string& query);
    ~WhereComparator() {}

    bool  checkCondition(const aq::AQMatrix& matrix, 
                         const std::map<size_t, std::vector<boost::shared_ptr<aq::ColumnMapper_Intf> > >& mapper,
                         size_t i)  const;

  private:
    static bool  checkJEQ(const aq::AQMatrix& matrix, 
                          const std::map<size_t, std::vector<boost::shared_ptr<aq::ColumnMapper_Intf> > >& mapper,
                          size_t i)  {return true;}
    static bool  checkJINF(const aq::AQMatrix& matrix, 
                           const std::map<size_t, std::vector<boost::shared_ptr<aq::ColumnMapper_Intf> > >& mapper,
                           size_t i) {return true;}
    static bool  checkJIEQ(const aq::AQMatrix& matrix, 
                           const std::map<size_t, std::vector<boost::shared_ptr<aq::ColumnMapper_Intf> > >& mapper,
                           size_t i) {return true;}
    static bool  checkJSUP(const aq::AQMatrix& matrix, 
                           const std::map<size_t, std::vector<boost::shared_ptr<aq::ColumnMapper_Intf> > >& mapper,
                           size_t i) {return true;}
    static bool  checkJSEQ(const aq::AQMatrix& matrix, 
                           const std::map<size_t, std::vector<boost::shared_ptr<aq::ColumnMapper_Intf> > >& mapper,
                           size_t i) {return true;}

  private:
    boost::function<bool (const aq::AQMatrix& matrix, 
                          const std::map<size_t, std::vector<boost::shared_ptr<aq::ColumnMapper_Intf> > >& mapper,
                          size_t i)>  _comparator;
  };

}

#endif // __WHERECOMPARATOR_H__