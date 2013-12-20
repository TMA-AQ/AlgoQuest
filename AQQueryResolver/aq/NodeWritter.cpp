#include "NodeWritter.h"
#include "TreeUtilities.h"

using namespace aq;

NodeWritter::NodeWritter(aq::tnode& _result)
   : result(_result), cur(&_result)
{
  aq::tnode::delete_subtree(result.left);
  aq::tnode::delete_subtree(result.right);
  aq::tnode::delete_subtree(result.next);
}

NodeWritter::~NodeWritter()
{
}

template <typename T>
aq::tnode * get_node(const aq::row_item_t::item_t& item)
{
  const auto& i = boost::get<aq::ColumnItem<T> >(item);
  return aq::util::GetNode<T>(i);
}

aq::tnode * get_node(const aq::row_item_t::item_t& item, aq::ColumnType type)
{
  aq::tnode * n = nullptr;
  switch (type)
  {
  case aq::ColumnType::COL_TYPE_BIG_INT:
  case aq::ColumnType::COL_TYPE_DATE:
    n = get_node<int64_t>(item);
    break;
  case aq::ColumnType::COL_TYPE_INT:
    n = get_node<int32_t>(item);
    break;
  case aq::ColumnType::COL_TYPE_DOUBLE:
    n = get_node<double>(item);
    break;
  case aq::ColumnType::COL_TYPE_VARCHAR:
    n = get_node<char*>(item);
    break;
  }
  return n;
}

int NodeWritter::process(std::vector<Row>& rows)
{
  for (const auto& row : rows)
  {
    assert(row.computedRow.size() == 1);
    const auto& item = row.computedRow[0].item;
    const auto& type = row.computedRow[0].type;

    if ((cur->getTag() == K_IN_VALUES) && (cur->right == nullptr))
    {
      // first value
      cur->right = get_node(item, type);
    }
    else
    {
      // other value
      aq::tnode * n = new aq::tnode(K_COMMA);
      n->left = cur->right;
      n->right = get_node(item, type);
      cur->right = n;
      cur = cur->right;
    }

    std::cout << result << std::endl;

  }

  return 0;
}

RowProcess_Intf * NodeWritter::clone()
{
  // TODO
  return nullptr;
}

const std::vector<Column::Ptr>& NodeWritter::getColumns() const
{
  return columns;
}

void NodeWritter::setColumn(std::vector<Column::Ptr> _columns)
{
  columns = _columns;
}

unsigned int NodeWritter::getTotalCount() const
{
  // TODO
  return 0;
}
