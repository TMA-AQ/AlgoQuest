#ifndef __DATABASE_HELPER_H__
#define __DATABASE_HELPER_H__

#include <string>
#include <memory>

#include "DatabaseGenerator.h"
#include <aq/Base.h>
#include <aq/Database.h>
#include <aq/Settings.h>
#include <aq/AQLQuery.h>

namespace aq {

  class DatabaseIntf
  {
  public:
    typedef std::list<std::list<std::string> > result_t;

    virtual ~DatabaseIntf() {}
    virtual void clean() = 0;
    virtual void createTable(const DatabaseGenerator::handle_t::tables_t::key_type& table) = 0;
    virtual void insertValues(const DatabaseGenerator::handle_t::tables_t::value_type& values) = 0;
    virtual bool execute(const aq::core::SelectStatement& ss, DatabaseIntf::result_t& r1) = 0;
  };

  class AlgoQuestDatabase : public DatabaseIntf
  {
  public:
    AlgoQuestDatabase(aq::Settings& _settings, bool _onlyEngine = false);
    void clean();
    void createTable(const DatabaseGenerator::handle_t::tables_t::key_type& table);
    void insertValues(const DatabaseGenerator::handle_t::tables_t::value_type& values);
    bool execute(const aq::core::SelectStatement& ss, DatabaseIntf::result_t& r1);
  private:
    aq::base_t               base;
    aq::Database             db;
    aq::Settings             settings;
    bool                     onlyEngine;
  };

}

#endif