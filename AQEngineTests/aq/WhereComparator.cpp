#include "WhereComparator.h"

namespace aq
{

  //-------------------------------------------------------------------------------

  WhereComparator::WhereComparator(const std::string& query) : AWhereCondition(query)
  {
    std::map<std::string, boost::function<bool (aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)> > mapper;

    mapper["K_JEQ"] = this->checkJEQ;
    mapper["K_JIEQ"] = this->checkJIEQ;
    mapper["K_JINF"] = this->checkJINF;
    mapper["K_JSEQ"] = this->checkJSEQ;
    mapper["K_JSUP"] = this->checkJSUP;

    std::string::size_type pos;

    for (auto& it : mapper)
      if ((pos = query.find(it.first)) != std::string::npos)
      {
        this->_comparator = it.second;
        break;
      }
  }

  //-------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------

    bool  WhereComparator::checkJEQ(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)
    {
      return aq::ColumnItem::equal(*item1, *item2);
    }

    bool  WhereComparator::checkJINF(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)
    {
      return aq::ColumnItem::lessThan(*item1, *item2);
    }

    bool  WhereComparator::checkJIEQ(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)
    {
      if (aq::ColumnItem::lessThan(*item1, *item2) == false)
        return aq::ColumnItem::equal(*item1, *item2);
      return true;
    }

    bool  WhereComparator::checkJSUP(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)
    {
      return aq::ColumnItem::lessThan(*item2, *item1);
    }

    bool  WhereComparator::checkJSEQ(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)
    {
      if (aq::ColumnItem::lessThan(*item2, *item1) == false)
        return aq::ColumnItem::equal(*item1, *item2);
      return true;
    }

  //-------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------

  aq::ColumnItem::Ptr     WhereComparator::addItem(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)
  {
    item1->numval += item2->numval;
    item1->strval += item2->strval;
    return item1;
  }

  aq::ColumnItem::Ptr     WhereComparator::subItem(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)
  {
    item1->numval -= item2->numval;
    return item1;
  }

  aq::ColumnItem::Ptr     WhereComparator::divItem(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)
  {
    assert(item2->numval != 0); // assert if division by 0
    item1->numval /= item2->numval;
    return item1;
  }

  aq::ColumnItem::Ptr     WhereComparator::mulItem(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)
  {
    item1->numval *= item2->numval;
    return item1;
  }

  aq::ColumnItem::Ptr     WhereComparator::modItem(aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)
  {
    item1->numval = static_cast<int>(item1->numval) % static_cast<int>(item2->numval);
    return item1;
  }

  aq::ColumnItem::Ptr           WhereComparator::calculCondition(std::vector<operateur> operateurs, std::vector<aq::ColumnItem::Ptr> items) const
  {
    assert(items.size() == operateurs.size() + 1);
    aq::ColumnItem::Ptr item = items.front();
    std::vector<aq::ColumnItem::Ptr>::iterator  it = items.begin();
    std::map<operateur, boost::function<aq::ColumnItem::Ptr (aq::ColumnItem::Ptr item1, aq::ColumnItem::Ptr item2)> > mapper;
    mapper[O_PLUS] = this->addItem;
    mapper[O_MOINS] = this->subItem;
    mapper[O_DIV] = this->divItem;
    mapper[O_FOIS] = this->mulItem;
    mapper[O_MOD] = this->modItem;
    for (auto& v : operateurs)
    {
      ++it;
      item = mapper[v](item, (*it));
    }
    return item;
  }

  //-------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------

  bool                    WhereComparator::checkCondition(const aq::AQMatrix& matrix, mapMap& mapper, size_t i) const
  {
    std::vector<aq::ColumnItem::Ptr> vect1, vect2;
    for (auto& t : matrix.getMatrix())
    {
      for (auto& v : this->_values.first.second)
      {
        if (t.table_id == v.first)
        {
          aq::ColumnItem::Ptr item(new aq::ColumnItem);
          mapper[t.table_id][v.second]->loadValue(t.indexes[i] - 1, *item);
          vect1.push_back(item);
        }
      }
      for (auto& v : this->_values.second.second)
      {
        if (t.table_id == v.first)
        {
          aq::ColumnItem::Ptr item(new aq::ColumnItem);
          mapper[t.table_id][v.second]->loadValue(t.indexes[i] - 1, *item);
          vect2.push_back(item);
        }
      }
    }
    aq::ColumnItem::Ptr item1 = this->calculCondition(this->_values.first.first, vect1);
    aq::ColumnItem::Ptr item2 = this->calculCondition(this->_values.second.first, vect2);
    return this->_comparator(item1, item2);
  }

  //-------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------

  std::vector<operateur>  WhereComparator::cutQuery(const std::vector<std::string>& vecteur, std::vector<pairString>& v2)
  {
    std::string s1, s2;
    std::vector<operateur> operateur;
    ++this->_pos;
    for (auto& v = (vecteur.begin() + this->_pos); v != vecteur.end(); ++v)
    {
      if ((*v) == ".")
      {
        ++v;
        s1 = (*v);
        ++v;
        s2 = (*v);
        v2.push_back(std::make_pair(s1, s2));
      }
      else if ((*v) == "+" || (*v) == "-" || (*v) == "*" || (*v) == "/" || (*v) == "%")
        operateur.push_back(this->castOperateur((*v)));
      if ((*v) == "K_INNER")
      {
        this->setPos(vecteur);
        return operateur;
      }
    }
    return operateur;
  }

  void                    WhereComparator::setPos(const std::vector<std::string>& vecteur)
  {
    size_t pos = this->_pos;
    for (auto& v = (vecteur.begin() + this->_pos); v != vecteur.end(); ++v)
    {
      if ((*v) == "K_INNER")
      {
        this->_pos = pos;
        return;
      }
      ++pos;
    }
  }

  void                    WhereComparator::setValues(const aq::Base& baseDesc)
  {
    this->_pos = 0;
    boost::to_upper(this->_query);
    std::vector<std::string> vecteur;
    boost::split(vecteur, this->_query, boost::is_any_of(" "));
    this->setPos(vecteur);
    std::vector<pairString> vecteur1, vecteur2;

    std::vector<operateur>  ope1, ope2;
    ope1 = this->cutQuery(vecteur, vecteur1);
    ope2 = this->cutQuery(vecteur, vecteur2);

    std::vector<pairSize> val1, val2;

    for (auto& v : vecteur1)
      val1.push_back(this->makePairSize(baseDesc, v));
    for (auto& v : vecteur2)
      val2.push_back(this->makePairSize(baseDesc, v));

    this->_values = std::make_pair(std::make_pair(ope1, val1), std::make_pair(ope2, val2));
  }

}