#include "DatabaseGenerator.h"
#include <cmath>
#include <numeric>
#include <cassert>
#include <boost/array.hpp>
#include <boost/random.hpp>

using namespace aq;

DatabaseGenerator::DatabaseGenerator(const std::list<std::string>& _tables, size_t _nb_rows, int _min_values, int _max_values, point_mode_t _mode, gen_mode_t _gen_mode, value_mode_t _value_mode)
  : nb_rows(_nb_rows), min_values(_min_values), max_values(_max_values), mode(_mode), gen_mode(_gen_mode), value_mode(_value_mode)
{
  for (const auto& table : _tables)
  {
    this->tables_bound.insert(std::make_pair(table, handle_t::values_t()));
  }
  this->it_table = this->tables_bound.begin();
}

DatabaseGenerator::~DatabaseGenerator()
{
}

size_t DatabaseGenerator::generate(handle_t * cb)
{
  size_t n = 0;
  if (it_table == tables_bound.end()) 
  {
    tables_values.clear();
    for (const auto& table : tables_bound)
    {
      assert(table.second.size() == 2);
      auto min = *table.second.begin();
      auto max = *table.second.rbegin();
      auto it = tables_values.insert(std::make_pair(table.first, handle_t::values_t()));
      if (!it.second)
      {
        // todo
        throw;
      }
      if ((this->value_mode == value_mode_t::ALL_UNIQUE) || (this->value_mode == value_mode_t::ALL_MULTIPLE))
      {
        for (auto v = min; v <= max; v++)
        {
          it.first->second.push_back(v);
          if (this->value_mode == value_mode_t::ALL_MULTIPLE)
            it.first->second.push_back(v);
        }
        std::random_shuffle(it.first->second.begin(), it.first->second.end());
      }
      else if (this->value_mode == value_mode_t::RANDOM)
      {
        boost::random::mt19937 rng;
        boost::random::uniform_int_distribution<> value(min,max);
        for (size_t i = 0; i < nb_rows; i++)
        {
          it.first->second.push_back(value(rng));
        }
      }
    }
    cb->push(this->tables_values);
    return 1;
  }
  std::vector<int> pts;
  if (it_table == tables_bound.begin())
  {
    it_table->second.push_back(min_values);
    it_table->second.push_back(max_values);
    it_table++;
  }

  for (auto it = tables_bound.begin(); it != it_table; ++it)
    for (const auto& pt : it->second)
      pts.push_back(pt);

  if (mode == point_mode_t::FULL)
  {
    std::sort(pts.begin(), pts.end());
    std::unique(pts.begin(), pts.end());
  }
  else if (mode == point_mode_t::MIN_MAX)
  {
    auto min = *std::min_element(pts.begin(), pts.end());
    auto max = *std::max_element(pts.begin(), pts.end());
    pts.clear();
    pts.push_back(min);
    pts.push_back(max);
  }
  else
  {
    assert(false);
  }

  std::list<std::pair<int, int> > l = aq::DatabaseGenerator::generate_list(pts, gen_mode);
  for (auto& values : l)
  {
    assert(it_table != tables_bound.end());
    it_table->second.clear();
    it_table->second.push_back(values.first);
    it_table->second.push_back(values.second);
    it_table++;
    n += this->generate(cb);
    it_table--;
  }
  return n;
}

size_t DatabaseGenerator::getNbDB() const
{
  size_t nb = 0;
  if ((mode == point_mode_t::FULL))
  {
    nb = 1;
    size_t b = 2;
    for (size_t i = 1; i < this->tables_bound.size(); i++)
    {
      nb *= (b) * (b + 1) / 2;
      b += 2;
    }
  }
  else if (mode == point_mode_t::MIN_MAX)
  {
    nb = (double)std::pow(gen_mode == gen_mode_t::ALL ? (double)15 : (double)6, (double)(this->tables_bound.size() - 1));
  }
  return nb;
}

boost::array<size_t, 2> getSlices(size_t n, size_t s)
{
  boost::array<size_t, 2> slices = { {s, n} };
  for (size_t j = 0; slices[0] <= slices[1]; j++)
  {
    slices[1] -= slices[0];
    slices[0] -= 1;
  }
  slices[0] = s - slices[0];
  slices[1] += slices[0];
  return slices;
}

std::list<std::pair<int, int> > DatabaseGenerator::generate_list(const std::vector<int>& pts, const gen_mode_t mode)
{
  std::list<std::pair<int, int> > t2_pts;
  if (pts.empty()) 
    return t2_pts;
  int v = 0;
  size_t len = 0;
  size_t l = (pts.size() > 1) ? (pts[1] - pts[0]) / 2: pts[0] / 2;
  size_t s = (2 * pts.size()) + 1;
  size_t n = s * (s + 1) / 2;
  for (size_t j = 0; j < n; j++)
  {
    size_t idx = 0;
    std::pair<int, int> pt(-1, -1);
    for (auto slice : getSlices(j, s))
    {
      assert(slice < s);
      idx = slice / 2;
      assert(idx <= pts.size());
      len = ((idx == pts.size()) || (slice <= 1)) ? l : ((slice % 2) == 0) ? (pts[idx] - pts[idx-1]) / 2 : 0;
      v = (idx == pts.size()) ? *pts.rbegin() : pts[idx];
      v = ((slice % 2) == 0) ? ((idx == pts.size()) ? v + len : v - len) : v;
      if (pt.first == -1) pt.first = v;
      else pt.second = v;
    }
    if (pt.first == pt.second)
    {
      pt.second = (idx < pts.size()) ? pts[idx] - 1 : v + l;
      if (pt.second < pt.first)
        pt.second = pt.first;
    }
    if (mode == gen_mode_t::ALL)
    {
      t2_pts.push_back(pt);
    }
    else if (mode == gen_mode_t::INTERSECT)
    {
      if (!(pt.first == pt.second))
      {
        bool belong = false;
        for (const auto& p : pts)
        {
          if ((p == pt.first) || (p == pt.second))
          {
            belong = true;
            break;
          }
        }
        if (!belong)
          t2_pts.push_back(pt);
      }
    }
  }
  return t2_pts;
}
