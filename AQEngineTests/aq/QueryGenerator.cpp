#include "QueryGenerator.h"
#include <algorithm>
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace aq;

void QueryGenerator::parse(std::istream& is, std::string& base, ops_t& ops, idents_t& idents)
{
  std::string line;
  while (std::getline(is, line))
  {
    boost::erase_all(line, "\n");
    boost::trim(line);
		if (line.find('=') != std::string::npos)
    {
      std::vector<std::string> op_2_values;
      std::vector<std::string> values;
      boost::split(op_2_values, line, boost::is_any_of("="));
      if (op_2_values.size() != 2)
        throw; // todo
      boost::trim(op_2_values[0]);
      boost::trim_if(op_2_values[1], boost::is_any_of(" []"));
			values = boost::split(values, op_2_values[1], boost::is_any_of(","));
			for (auto& v : values)
        boost::trim_if(v, boost::is_any_of(" '"));
			ops.insert(std::make_pair(op_2_values[0], values));
    }
		else if (line != "")
    {
      base += line + "\n";
    }
		if (line.find(';') != std::string::npos)
			break;
  }

	for (const auto& op : ops)
  {
    std::string s("\\[" + op.first + "_[0-9]+\\]");
    boost::regex regex(s);
    boost::sregex_token_iterator iter(base.begin(), base.end(), regex, 0);
    boost::sregex_token_iterator end;
    while (iter != end)
    {
      auto& s = *iter;
      idents.push_back(s);
      ++ iter;
    }
  }
}

QueryGenerator::QueryGenerator(std::istream& is)
{
  idents_t idents;
  QueryGenerator::parse(is, this->base, this->ops, idents);
  this->initIdents(idents);
}

QueryGenerator::QueryGenerator(const std::string& _base, const ops_t& _ops, const idents_t& _idents)
  : base(_base), ops(_ops)
{
  this->initIdents(_idents);
}

void QueryGenerator::initIdents(const idents_t& idents)
{
  for (auto& ident : idents)
  {
    for (auto& op_values : ops)
    {
      if (ident.find(op_values.first) != std::string::npos)
      {
        values.insert(std::make_pair(ident, idents_t()));
        auto& vs = values.rbegin()->second;
        if (ident.substr(0, 5) == "[MIX_")
        {
          std::string comma;
          for (size_t i = 0; i < op_values.second.size(); i++)
            comma += ", ";
          do
          {
            std::string perm;
            for (const auto& v : op_values.second) 
              perm += v + " ";
            vs.push_back(comma + perm);
          }
          while (std::next_permutation(op_values.second.begin(), op_values.second.end()));
        }
        else
        {
          for (const auto& v : op_values.second)
            vs.push_back(v);
        }
      }
    }
  }
}

QueryGenerator::~QueryGenerator()
{
}

void QueryGenerator::reset()
{
  for (auto& v : values)
  {
    this->resetValue(v);
  }
}

void QueryGenerator::resetValue(ops_t::value_type& value)
{   
  value.second.clear();
  std::string op = value.first;
  boost::trim_if(op, boost::is_any_of("[]"));
  std::string::size_type pos = op.find("_");
  if (pos != std::string::npos)
  {
    op = op.substr(0, pos);
  }
  const auto& vs = ops.find(op);
  assert(vs != ops.end());
  value.second = vs->second;
}

std::string QueryGenerator::next()
{
  if (this->values.empty() || this->values.rbegin()->second.empty())
    return "";
  std::string aql_query = this->base;
  bool unpack = true;
  auto last = values.end();
  --last;
  for (auto it = values.begin(); it != values.end(); ++it)
  {
    boost::replace_all(aql_query, it->first, *it->second.rbegin());
    if ((unpack) && ((it->second.size() > 1) || (it == last)))
    {
      it->second.pop_back();
      if (it != values.begin())
      {
        auto it2 = it;
        do
        {
          --it2;
          this->resetValue(*it2);
          //it2->second.clear();
          //std::string op = it2->first;
          //boost::trim_if(op, boost::is_any_of("[]"));
          //std::string::size_type pos = op.find("_");
          //if (pos != std::string::npos)
          //{
          //  op = op.substr(0, pos);
          //}
          //const auto& vs = ops.find(op);
          //assert(vs != ops.end());
          //it2->second = vs->second;
        } while (it2 != values.begin());
      }
      unpack = false;
    }
  }
  return aql_query;
}

size_t QueryGenerator::getNbQueries() const
{
  size_t nb = 1;
  for (const auto& value : values)
  {
    size_t n = 0;
    std::string::size_type pos = 0;
    while ((pos = base.find(value.first, pos)) != std::string::npos)
    {
      n += 1;
      pos += value.first.size();
    }
    nb *= n * value.second.size();
  }
  return nb;
}
