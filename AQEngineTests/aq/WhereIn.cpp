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
    std::vector<std::string> vecter;
    std::vector<std::string> vecter2;
    std::vector<int> vecter1;
    boost::split(vecter2, this->_query, boost::is_any_of(" "));
    for (auto& v = vecter2.begin(); v != vecter2.end(); ++v)
    {
      if (v->find("TABLE") != std::string::npos)
      {
        vecter.push_back((*v));
        ++v;
        vecter.push_back((*v));
        break;
      }
    }
    
    for (auto& v = vecter2.begin(); v != vecter2.end(); ++v)
    {
      if (v->find("K_VALUE") != std::string::npos)
      {
        ++v;
        vecter1.push_back(atoi((*v).c_str()));
      }
    }

    size_t t1 = 0, t2 = 0;
    for (auto& it = baseDesc.getTables().begin(); it != baseDesc.getTables().end(); ++it)
    {
      for (auto& v = vecter.begin(); v != vecter.end(); ++v)
      {
        if ((*it)->getName() == (*v))
        {
          ++v;
          for (auto& it2 = (*it)->Columns.begin(); it2 != (*it)->Columns.end(); ++it2)
          {
            if ((*it2)->getName() == (*v))
            {
              t1 = (*it)->ID;
              t2 = (*it2)->ID;
            }
          }
        }
      }
    }

    this->_values = std::make_pair(std::make_pair(t1, t2), vecter1);
  }

}