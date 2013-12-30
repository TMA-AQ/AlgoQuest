#ifndef __ROW_SOLVER_H__
#define __ROW_SOLVER_H__

#include "AQMatrix.h"
#include "RowProcess_Intf.h"
#include "ColumnMapper_Intf.h"
#include "parser/SQLParser.h"
#include <aq/BaseDesc.h>
#include <aq/Base.h>
#include <aq/Table.h>
#include <aq/Column.h>
#include <vector>
#include <boost/tuple/tuple.hpp>
#include <boost/thread/mutex.hpp>

namespace aq {

class RowSolver
{
public:
  struct column_infos_t
  {
    typedef boost::variant<
      aq::ColumnMapper_Intf<int32_t>::Ptr, 
      aq::ColumnMapper_Intf<int64_t>::Ptr, 
      aq::ColumnMapper_Intf<double>::Ptr, 
      aq::ColumnMapper_Intf<char>::Ptr 
    > mapper_type_t;

    column_infos_t() : table_index(0), grouped(false) {}
    Column::Ptr column; 
    mapper_type_t mapper; 
    size_t table_index; 
    bool grouped;
  };
  typedef std::vector<column_infos_t> columns_infos_t;
  

public:
  RowSolver(boost::shared_ptr<aq::AQMatrix> _aqMatrix, const std::vector<Column::Ptr>& _columnTypes, 
    const std::vector<aq::tnode*> _columnGroup, const Settings& _settings, const Base& _BaseDesc);

  void solve(boost::shared_ptr<aq::RowProcess_Intf> rowProcess, uint64_t nbThread, bool aggregate = false);

protected:

  void matched_index(column_infos_t& infos);

  void solve_thread(
    boost::shared_ptr<aq::RowProcess_Intf> rowProcess,
    const std::pair<size_t, size_t> indexes,
    const columns_infos_t columns,
    const bool aggregate);

  void addGroupColumn(
    const std::vector<Column::Ptr>& columnTypes, 
    const std::vector<aq::tnode*>& columnGroup,
    columns_infos_t& columns_infos);

  void prepareColumnAndColumnMapper(
    const std::vector<Column::Ptr>& columnTypes, 
    const std::vector<aq::tnode*>& columnGroup,
    columns_infos_t& columns_infos);

private:
  boost::mutex mutex;
  boost::shared_ptr<aq::AQMatrix> aqMatrix; 
  const std::vector<Column::Ptr>& columnTypes; 
  const std::vector<aq::tnode*> columnGroup;
  const Settings& settings;
  const Base& BaseDesc;
  boost::shared_ptr<aq::RowProcess_Intf> rowProcess;
  uint64_t nbThread;
  bool aggregate;
};

}

#endif