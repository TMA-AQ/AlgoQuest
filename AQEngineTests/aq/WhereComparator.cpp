#include "WhereComparator.h"

namespace aq
{
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

  bool  WhereComparator::checkCondition(const aq::AQMatrix& matrix, mapMap& mapper, size_t i) const
  {
    aq::ColumnItem::Ptr item1(new aq::ColumnItem);
    aq::ColumnItem::Ptr item2(new aq::ColumnItem);
    for (auto& t : matrix.getMatrix())
    {
      if (t.table_id == this->_values.first.first)
      {
        mapper[t.table_id][this->_values.first.second]->loadValue(t.indexes[i] - 1, *item1);
      }
      if (t.table_id == this->_values.second.first)
      {
        mapper[t.table_id][this->_values.second.second]->loadValue(t.indexes[i] - 1, *item2);
      }
    }
    return this->_comparator(item1, item2);
  }

  void  WhereComparator::setValues(const aq::Base& baseDesc) // a refaire de façon plus generique

  {
    boost::to_upper(this->_query);
    std::vector<std::string> vecter;
    std::vector<std::string> vecter2;
    boost::split(vecter2, this->_query, boost::is_any_of(" "));
    for (auto& v = vecter2.begin(); v != vecter2.end(); ++v)
    {
      if (v->find("TABLE") != std::string::npos)
      {
        vecter.push_back((*v));
        ++v;
        vecter.push_back((*v));
      }
    }

    bool trash = true;
    size_t t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    for (auto& it = baseDesc.getTables().begin(); it != baseDesc.getTables().end(); ++it)
    {
      for (auto& v = vecter.begin(); v != vecter.end(); ++v)
      {
        if ((*it)->getName() == (*v))
        {
          ++v;
          for (auto& it2 = (*it)->Columns.begin(); it2 != (*it)->Columns.end(); ++it2)
          {
            if ((*it2)->getName() == (*v) && trash == true)
            {
              t1 = (*it)->ID;
              t2 = (*it2)->ID;
              trash = false;
            }
            else if ((*it2)->getName() == (*v))
            {
              t3 = (*it)->ID;
              t4 = (*it2)->ID;
            }
          }
        }
      }
    }
    this->_values = std::make_pair(std::make_pair(t1, t2), std::make_pair(t3, t4));
  }

}