#ifndef __WHERECOMPARATOR_H__
# define __WHERECOMPARATOR_H__

# include <boost/function.hpp>

# include "AWhereCondition.h"

namespace aq
{

  typedef std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t> >          pairPair;

  class WhereComparator : public AWhereCondition
  {
  public:
    WhereComparator(const std::string& query);
    ~WhereComparator() {}

    bool  checkCondition(const aq::AQMatrix& matrix, mapMap& mapper, size_t i)  const;
    void  setValues(const aq::Base& baseDesc);

  private:
    static bool  checkJEQ(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)
    {
      if ((item1->numval == item2->numval) && (item1->strval == item2->strval))
        return true;
      return false;
    }

    static bool  checkJINF(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)
    {
      if ((item1->numval < item2->numval) ||  (item1->strval < item2->strval))
        return true;
      return false;
    }

    static bool  checkJIEQ(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)
    {
      if (((item1->numval < item2->numval) ||  (item1->strval < item2->strval)) == false)
        return checkJEQ(item1, item2);
      return true;
    }

    static bool  checkJSUP(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)
    {
      if ((item2->numval < item1->numval) ||  (item2->strval < item1->strval))
        return true;
      return false;
    }

    static bool  checkJSEQ(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)
    {
      if (((item2->numval < item1->numval) ||  (item2->strval < item1->strval)) == false)
        return checkJEQ(item1, item2);;
      return true;
    }

  private:
    boost::function<bool (aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)>  _comparator;
    pairPair                                                                      _values;
  };

}

#endif // __WHERECOMPARATOR_H__