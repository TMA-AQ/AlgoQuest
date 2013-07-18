#ifndef __WHEREIN_H__
# define __WHEREIN_H__

# include "AWhereCondition.h"

namespace aq
{

  typedef std::pair<std::pair<size_t, size_t>, std::vector<int> > pairVector;

  class WhereIn : public AWhereCondition
  {
  public:
    WhereIn(const std::string& query);
    ~WhereIn() {}

    bool  checkCondition(const aq::AQMatrix& matrix, mapMap& mapper, size_t i) const;

    void  setValues(const aq::Base& baseDesc);

  private:
    pairVector  _values;
  };

}

#endif // __WHEREIN_H__