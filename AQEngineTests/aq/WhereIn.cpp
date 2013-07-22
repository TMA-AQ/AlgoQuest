#include "WhereIn.h"

namespace aq
{

  WhereIn::WhereIn(const std::string& query) : AWhereCondition(query)
  {}

  bool  WhereIn::checkCondition(const aq::AQMatrix& matrix, mapMap& mapper, size_t i) const
  {
    aq::ColumnItem::Ptr item(new aq::ColumnItem);
    for (auto& t : matrix.getMatrix())
    {
      if (t.table_id == this->_values.first.first)
        mapper[t.table_id][this->_values.first.second]->loadValue(t.indexes[i] - 1, *item);
    }

    bool  check = false;
    for (auto& t : this->_values.second)
    {
      if (t == item->numval)
        check = true;
    }
    return check;
  }

  void  WhereIn::setValues(const aq::Base& baseDesc)
  {
    boost::to_upper(this->_query);
    std::string s1, s2;
    std::vector<std::string> vecter2;
    std::vector<int> vecter1;
    boost::split(vecter2, this->_query, boost::is_any_of(" "));
    for (auto& v = vecter2.begin(); v != vecter2.end(); ++v)
    {
      if (v->find("TABLE") != std::string::npos)
      {
        s1 = (*v);
        ++v;
        s2 = (*v);
        break;
      }
    }
    pairString tabVal = std::make_pair(s1, s2);
    for (auto& v = vecter2.begin(); v != vecter2.end(); ++v)
    {
      if (v->find("K_VALUE") != std::string::npos)
      {
        ++v;
        vecter1.push_back(atoi((*v).c_str()));
      }
    }

    pairSize val = this->makePairSize(baseDesc, tabVal);
    this->_values = std::make_pair(val, vecter1);
  }

}