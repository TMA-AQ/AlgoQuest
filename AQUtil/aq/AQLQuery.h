#ifndef __AQ_CORE_QUERY_HPP__
#define __AQ_CORE_QUERY_HPP__

#include <string>
#include <vector>

namespace aq {
namespace core {

struct TableReference
{
  std::string name;
};

struct ColumnReference
{
  TableReference table;
  std::string name;
};

struct InCondition
{
  ColumnReference column;
  std::vector<std::string> values;
};

struct JoinCondition
{
  enum join_t
  {
    INNER,
    LEFT_OUTER,
    RIGHT_OUTER,
    FULL_OUTER,
  };

  enum op_t
  {
    AUTO,
    EQ,
    NEQ,
    INF,
    IEQ,
    SEQ,
    SUP,
  };
  
  std::string op;
  std::string jt_left, jt_right;
  ColumnReference left, right;
};

struct SelectStatement
{
  enum output_t
  {
    AQL, 
    SQL,
  };
  output_t output;
  inline void setOutput(output_t o) { this->output = o; }
  std::string to_string() const;
  void to_string(std::string& str) const;

  std::vector<ColumnReference> selectedTables;
  std::vector<TableReference> fromTables;
  std::vector<JoinCondition> joinConditions;
  std::vector<InCondition> inConditions;
  std::vector<ColumnReference> groupedColumns, orderedColumns;

};

}
}

std::ostream& operator<<(std::ostream& os, aq::core::TableReference const& ss);
std::ostream& operator<<(std::ostream& os, aq::core::ColumnReference const& ss);
std::ostream& operator<<(std::ostream& os, aq::core::SelectStatement const& ss);

bool operator==(const aq::core::TableReference& tr1, const aq::core::TableReference& tr2);
bool operator==(const aq::core::ColumnReference& cr1, const aq::core::ColumnReference& cr2);
bool operator==(const aq::core::InCondition& ic1, const aq::core::InCondition& ic2);
bool operator==(const aq::core::JoinCondition& jc1, const aq::core::JoinCondition& jc2);

#endif