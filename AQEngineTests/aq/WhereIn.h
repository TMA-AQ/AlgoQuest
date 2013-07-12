#ifndef __WHEREIN_H__
# define __WHEREIN_H__

# include "AWhereCondition.h"

namespace aq
{

  class WhereIn : public AWhereCondition
  {
  public:
    WhereIn(const std::string& query);
    ~WhereIn() {}

    bool  checkCondition(const aq::AQMatrix& matrix, 
                         const std::map<size_t, std::vector<boost::shared_ptr<aq::ColumnMapper_Intf> > >& mapper,
                         size_t i) const {return true;}
  };

}

#endif // __WHEREIN_H__