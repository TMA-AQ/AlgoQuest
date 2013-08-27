#ifndef __WHERECOMPARATOR_H__
# define __WHERECOMPARATOR_H__

# include <boost/function.hpp>

# include "AWhereCondition.h"

namespace aq
{

  typedef std::pair<std::vector<operateur>, std::vector<pairSize> >   pairVector;
  typedef std::pair<pairVector, pairVector>                           pairPair;
  typedef std::vector<pairString>                                     vectorPair;

  class WhereComparator : public AWhereCondition
  {
  public:

    //-------------------------------------------------------------------------
    // Constructor/Destructor
    //-------------------------------------------------------------------------

    WhereComparator(const std::string& query);
    ~WhereComparator() {}

    //-------------------------------------------------------------------------
    // assign values and check it
    //-------------------------------------------------------------------------

    bool  checkCondition(const aq::AQMatrix& matrix, mapMap& mapper, size_t i)  const;
    void  setValues(const aq::Base& baseDesc);

  private:
    
    //-------------------------------------------------------------------------
    // assign _values and check them
    //-------------------------------------------------------------------------

    static bool  checkJEQ(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2);
    static bool  checkJINF(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2);
    static bool  checkJIEQ(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2);
    static bool  checkJSUP(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2);
    static bool  checkJSEQ(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2);

  private:
    
    //-------------------------------------------------------------------------
    // tools to set _values
    //-------------------------------------------------------------------------

    std::vector<operateur>  cutQuery(const std::vector<std::string>& v1, std::vector<pairString>& v2);
    void                    setPos(const std::vector<std::string>& v1);

  private:
    
    //-------------------------------------------------------------------------
    // calcul Item in case of any kind of operator
    //-------------------------------------------------------------------------

    aq::ColumnItem::Ptr         calculCondition(std::vector<operateur> operateurs, std::vector<aq::ColumnItem::Ptr> items)  const;
    static aq::ColumnItem::Ptr  addItem(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2);
    static aq::ColumnItem::Ptr  subItem(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2);
    static aq::ColumnItem::Ptr  divItem(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2);
    static aq::ColumnItem::Ptr  mulItem(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2);
    static aq::ColumnItem::Ptr  modItem(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2);


  private:
    
    //-------------------------------------------------------------------------
    // attributs
    //-------------------------------------------------------------------------

    size_t                                                                        _pos;
    boost::function<bool (aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)>  _comparator;
    pairPair                                                                      _values;
  };

}

#endif // __WHERECOMPARATOR_H__