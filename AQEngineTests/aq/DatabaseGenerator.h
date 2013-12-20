#ifndef __AQ_DATABASE_GENERATOR_H__
#define __AQ_DATABASE_GENERATOR_H__

#include <string>
#include <vector>
#include <list>
#include <map>

namespace aq
{

  class DatabaseGenerator
  {
  public:
    enum class point_mode_t
    {
      FULL,
      MIN_MAX,
    };
    enum class gen_mode_t
    {
      ALL,
      INTERSECT,
    };
    enum class value_mode_t
    {
      ALL_UNIQUE,
      ALL_MULTIPLE,
      RANDOM,
    };
    struct handle_t
    {
      typedef std::vector<int> values_t;
      typedef std::map<std::string, values_t> tables_t;
      virtual ~handle_t() {}
      virtual void push(const tables_t& tables) = 0;
    };
  public:
    DatabaseGenerator(const std::list<std::string>& tables, size_t nb_rows, int min_values, int max_values, point_mode_t mode, gen_mode_t gen_mode, value_mode_t _value_mode);
    ~DatabaseGenerator();
    size_t generate(handle_t * cb);
    size_t getNbDB() const;
  private:
    DatabaseGenerator(const DatabaseGenerator&);
    DatabaseGenerator& operator=(const DatabaseGenerator&);

    static std::list<std::pair<int, int> > generate_list(const std::vector<int>& pts, const gen_mode_t mode = gen_mode_t::ALL);
    
    handle_t::tables_t tables_bound;
    handle_t::tables_t tables_values;
    handle_t::tables_t::iterator it_table;
    size_t nb_rows;
    int min_values;
    int max_values;
    point_mode_t mode;
    gen_mode_t gen_mode;
    value_mode_t value_mode;
  };

}

#endif