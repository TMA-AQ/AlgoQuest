#include "DatabaseHelper.h"
#include "Util.h"

#include <aq/Logger.h>
#include <aq/Database.h>
#include <aq/db_loader/DatabaseLoader.h>

#include <aq/Base.h>
#include <aq/AQEngine.h>
#include <aq/RowWritter_Intf.h>
#include <aq/QueryResolver.h>

#include <fstream>

using namespace aq;

AlgoQuestDatabase::AlgoQuestDatabase(aq::Settings& _settings, bool _onlyEngine)
  : db(_settings.rootPath), settings(_settings), onlyEngine(_onlyEngine)
{
  base.id = 1; // fixme
  base.name = settings.rootPath;
  boost::replace_all(base.name, "\\", "/");
  while (!base.name.empty() && (*base.name.rbegin() == '/'))
    base.name.erase(base.name.size() - 1);
  std::string::size_type pos = base.name.find_last_of("/");
  if ((pos != std::string::npos) && (pos < base.name.size() - 1))
    base.name = base.name.substr(pos + 1);
  db.erase();
}

void AlgoQuestDatabase::clean()
{
  db.erase();
  base.table.clear();
}

void AlgoQuestDatabase::createTable(const DatabaseGenerator::handle_t::tables_t::key_type& table)
{ 
  base.table.push_back(base_t::table_t());
  auto& t = *base.table.rbegin();
  t.id = base.table.size();
  t.name = table;
  t.nb_record = 0;
  t.colonne.push_back(base_t::table_t::col_t());
  t.colonne.push_back(base_t::table_t::col_t());
  t.colonne[0].id = 1;
  t.colonne[1].id = 2;
  t.colonne[0].name = "id";
  t.colonne[1].name = "v1";
  t.colonne[0].size = 1;
  t.colonne[1].size = 1;
  t.colonne[0].type = aq::symbole::t_int;
  t.colonne[1].type = aq::symbole::t_int; 
  db.create(base);
}

void AlgoQuestDatabase::insertValues(const DatabaseGenerator::handle_t::tables_t::value_type& values)
{
  // write columns
  size_t base_id = 1;
  size_t table_id = 0;
  size_t column_id = 1;
  size_t packet = 0;
  char column_filename[1024];
  ::memset(column_filename, 0, 1024);
   
  for (const auto& table : base.table)
  {
    if (table.name == values.first)
    {
      table_id = table.id;
    }
  }
  if (table_id == 0)
  {
    // todo
    throw;
  }

  sprintf(column_filename, "%s/B%.3uT%.4uC%.4uP%.12u", db.getTemporaryColumnLoadPath().c_str(), base_id, table_id, column_id, packet);
  std::ofstream c1file(column_filename, std::ios::binary);
  for (int32_t v = 1; v <= values.second.size(); v++)
  {
    c1file.write(reinterpret_cast<const char*>(&v), sizeof(int32_t));
  }
  c1file.close();
  column_id += 1;
  sprintf(column_filename, "%s/B%.3uT%.4uC%.4uP%.12u", db.getTemporaryColumnLoadPath().c_str(), base_id, table_id, column_id, packet);
  std::ofstream c2file(column_filename, std::ios::binary);
  for (const auto& value : values.second)
  {
    c2file.write(reinterpret_cast<const char*>(&value), sizeof(int));
  }
  c2file.close();

  // call algoquest loader
  aq::DatabaseLoader loader(base, "aq-loader", db.getRootPath(), aq::packet_size, ',', true); // FIXME
  loader.generate_ini();
  loader.loadAllColumns();

  base.table[table_id - 1].nb_record = values.second.size();
  db.create(base);
}

class ResultHandler : public aq::RowWritter_Intf
{
public:
  ResultHandler(DatabaseIntf::result_t& _result) : result(_result) {}
  
  RowProcess_Intf * clone() { return nullptr; } 
  const std::vector<Column::Ptr>& getColumns() const { return columns; }
  void setColumn(std::vector<Column::Ptr> _columns) {}
  unsigned int getTotalCount() const { return 0; }
  
  int process(std::vector<Row>& rows)
  {
    for (const auto& row : rows)
    {
      result.push_back(DatabaseIntf::result_t::value_type());
      auto& r = *result.rbegin();
      if (row.completed)
      {
        std::string value;
        for (aq::Row::row_t::const_reverse_iterator it = row.computedRow.rbegin(); it != row.computedRow.rend(); ++it)
        {
          if ((*it).null)
          {
            value = "NULL";
          }
          else
          {
            switch((*it).type)
            {
            case aq::ColumnType::COL_TYPE_INT:
              value = boost::get<aq::ColumnItem<int32_t> >((*it).item).toString();
              break;
            case aq::ColumnType::COL_TYPE_DOUBLE:
              value = boost::get<aq::ColumnItem<double> >((*it).item).toString();
              break;
            case aq::ColumnType::COL_TYPE_DATE:
            case aq::ColumnType::COL_TYPE_BIG_INT:
              value = boost::get<aq::ColumnItem<int64_t> >((*it).item).toString();
              break;
            case aq::ColumnType::COL_TYPE_VARCHAR:
              value = boost::get<aq::ColumnItem<char*> >((*it).item).toString();
              break;
            }
          }
          r.push_back(value);
        }
      }
    }
    return 0;
  }

private:
  std::vector<Column::Ptr> columns;
  DatabaseIntf::result_t& result;
};

struct result_handler_t : public aq::display_cb
{
  result_handler_t(DatabaseIntf::result_t& _result)
    : result(_result)
  {
    it = result.rend();
  }
  ~result_handler_t()
  {
    it = result.rend();
  }
  void push(const std::string& value)
  {
    if (it != result.rend())
      (*it).push_back(value);
  }
  void next()
  {
    result.push_back(DatabaseIntf::result_t::value_type());
    it = result.rbegin();
  }
  DatabaseIntf::result_t& result;
  DatabaseIntf::result_t::reverse_iterator it;
};

bool AlgoQuestDatabase::execute(const aq::core::SelectStatement& ss, DatabaseIntf::result_t& result)
{
  if (onlyEngine)
  {
    try
    {
      aq::Base base(settings.dbDesc);
      aq::AQEngineSystem engine(base, settings);
      engine.prepare();
      engine.call(ss);
      engine.clean();
      auto matrix = engine.getAQMatrix();
      if (matrix != nullptr)
      {
        boost::shared_ptr<aq::display_cb> cb(new result_handler_t(result)); 
        std::vector<std::string> columns;
        aq::opt o;
        o.withCount = o.withIndex = false;
        for (const auto& c : ss.selectedTables)
          columns.push_back(c.table.name + "." + c.name);
        aq::display(cb.get(), *matrix, base, settings, o, columns);
      }
    }
    catch (const aq::generic_error& ge)
    {
      aq::Logger::getInstance().log(AQ_NOTICE, "%s\n", ge.what());
      result.clear();
    }
  }
  else
  {
    std::string         query;
    aq::Base            bd;
    aq::AQEngine_Intf * aqEngine = nullptr;
    const std::string   ident; 
    bool                keepFiles = false;
    bool                force = false;

    aq::Base::load(settings.dbDesc, bd);
    aqEngine = new aq::AQEngineSystem(bd, settings);
    // aqEngine = new aq::AQEngineWindows(bd, settings);

    ss.setOutput(aq::core::SelectStatement::output_t::SQL);
    ss.to_string(query);

    if (query.find("full outer join") != std::string::npos)
    {
      return true;
    }

    result.clear();
    boost::shared_ptr<aq::RowWritter_Intf> resultHandler(new ResultHandler(result));
    aq::QueryResolver::prepareQuery(query, ident, settings, force);
    aq::QueryResolver::processQuery(query, settings, bd, aqEngine, resultHandler, keepFiles);

    delete aqEngine;
  }
  return true;
}
