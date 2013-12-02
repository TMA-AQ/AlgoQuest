#ifndef __AQ_EXPRTRANSFORM_H__
#define __AQ_EXPRTRANSFORM_H__

#include "parser/SQLParser.h"
#include "parser/sql92_grm_tab.hpp"
#include "Column2Table.h"
#include "ColumnMapper_Intf.h"
#include "TreeUtilities.h"
#include "ThesaurusReader.h"
#include <aq/Exceptions.h>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>
#include <boost/static_assert.hpp>

namespace 
{
  
  // ----------------------------------------------------------------------------
  template <typename T>
  void nodeToItem(const aq::tnode&, aq::ColumnItem<T>&)
  {
    BOOST_STATIC_ASSERT_MSG(sizeof(T) == 0, "missing specialisation");
  }
  
  template <> inline
  void nodeToItem<int32_t>(const aq::tnode& node, aq::ColumnItem<int32_t>& item)
  {
    item.setValue(static_cast<int32_t>(node.getData().val_int));
  }
  
  template <> inline
  void nodeToItem<int64_t>(const aq::tnode& node, aq::ColumnItem<int64_t>& item)
  {
    item.setValue(static_cast<int64_t>(node.getData().val_int));
  }
  
  template <> inline
  void nodeToItem<double>(const aq::tnode& node, aq::ColumnItem<double>& item)
  {
    item.setValue(node.getData().val_number);
  }
  
  template <> inline
  void nodeToItem<char*>(const aq::tnode& node, aq::ColumnItem<char*>& item)
  {
    item.setValue(node.getData().val_str);
  }
  
  // ----------------------------------------------------------------------------
  template <typename T>
  aq::tnode * itemToNode(const aq::ColumnItem<T>&, const aq::ColumnType)
  {
    BOOST_STATIC_ASSERT_MSG(sizeof(T) == 0, "missing specialisation");
  }
  
  template <> inline
  aq::tnode * itemToNode<int32_t>(const aq::ColumnItem<int32_t>& item, const aq::ColumnType type)
  {
    aq::tnode * n = new aq::tnode(K_INTEGER);
    n->set_int_data(item.getValue());
    return n;
  }

  template <> inline
  aq::tnode * itemToNode<int64_t>(const aq::ColumnItem<int64_t>& item, const aq::ColumnType type)
  {
    aq::tnode * n = new aq::tnode(K_INTEGER);
    n->set_int_data(item.getValue());
    return n;
  }

  template <> inline
  aq::tnode * itemToNode<double>(const aq::ColumnItem<double>& item, const aq::ColumnType type)
  {
    aq::tnode * n = new aq::tnode(K_REAL);
    n->set_double_data(item.getValue());
    return n;
  }

  template <> inline
  aq::tnode * itemToNode<char*>(const aq::ColumnItem<char*>& item, const aq::ColumnType type)
  {
    aq::tnode * n = new aq::tnode(K_STRING);
    n->set_string_data(item.getValue());
    return n;
  }

  template <typename T>
  aq::tnode* create_in_subtree(const std::vector<aq::ColumnItem<T> >& items, const aq::ColumnType type, size_t nLevel = 0)
  {	
    aq::tnode *pNode = new aq::tnode(K_IN);

    if (items.empty())
      return nullptr;

    if (items.size() < 2)
    {
      pNode->right = itemToNode(items[0], type);
    }
    else
    {
      aq::tnode * n = new aq::tnode(K_COMMA);
      pNode->right = n;
      for (size_t i = 0; i < items.size() - 2; ++i)
      {
        n->left = itemToNode(items[i], type);
        n->right = new aq::tnode(K_COMMA);
        n = n->right;
      }
      n->left = itemToNode(*(items.rbegin() + 1), type);
      n->right = itemToNode(*items.rbegin(), type);
    }

    return pNode;
  }

  template <typename T>
  aq::tnode* create_eq_subtree(const aq::ColumnItem<T>& item, const aq::ColumnType type)
  {
    aq::tnode * n = new aq::tnode(K_EQ);
    switch (type)
    {
    case aq::ColumnType::COL_TYPE_INT:
    case aq::ColumnType::COL_TYPE_BIG_INT:
    case aq::ColumnType::COL_TYPE_DATE:
      n->right = new aq::tnode(K_INTEGER);
      n->right->set_int_data(static_cast<llong>(item.numval));
      break;
    case aq::ColumnType::COL_TYPE_DOUBLE:
      n->right = new aq::tnode(K_REAL);
      n->right->set_double_data(item.numval);
      break;
    case aq::ColumnType::COL_TYPE_VARCHAR:
      n->right = new aq::tnode(K_STRING);
      n->right->set_string_data(item.strval.c_str());
      break;
    }
    assert(n != nullptr);
    return n;
  }
  
  template <typename T>
  aq::tnode * getResult(const std::vector<aq::ColumnItem<T> >& result, const aq::ColumnType& type)
  {
    if (result.empty()) 
    {
      return new aq::tnode( K_FALSE );
    } 
    else 
    {
      return ::create_in_subtree(result, type);
    }
  }

  void getColumnInfos(const aq::Base& baseDesc, const aq::tnode& node, size_t& tId, size_t& cId, size_t& cSize, aq::ColumnType& cType)
  {
    std::string tableName = node.left->getData().val_str;
    std::string columnName = node.right->getData().val_str;
    boost::trim(tableName);
    boost::trim(columnName);
    boost::to_upper(tableName);
    boost::to_upper(columnName);
    aq::Table::Ptr table = baseDesc.getTable(tableName);
    aq::Column::Ptr column;
    for (auto& col : table->Columns)
    {
      if (col->getName() == columnName)
      {
        // assert(col->TableID == table->ID);
        tId = table->ID;
        cId = col->ID;
        cSize = col->Size;
        cType = col->Type;
        column = col;
        break;
      }
    }
    if (!column)
    {
      throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
    }
  }

  bool check_between_for_transform(const aq::tnode& n) 
  {
    return (((n.tag == K_BETWEEN) || (n.tag == K_NOT_BETWEEN)) && 
      aq::util::is_column_reference(n.left) && 
      n.right && (n.right->tag == K_AND) && 
      n.right->left && ((n.right->left->tag == K_STRING) || (n.right->left->tag == K_INTEGER)) && 
      n.right->right && ((n.right->right->tag == K_STRING) || (n.right->right->tag == K_INTEGER))); 
  }

  bool check_like_for_transform(const aq::tnode& n) 
  {
    return (((n.tag == K_LIKE) || (n.tag == K_NOT_LIKE)) &&
      (aq::util::is_column_reference(n.left)) && 
      n.right && 
      ((n.right->tag == K_STRING) || 
       ((n.right->tag == K_ESCAPE) && ((n.right->left) && (n.right->left->tag == K_STRING) && 
        (n.right->right) && (n.right->right->tag == K_STRING)))));
  }

  bool check_cmp_op_for_transform(const aq::tnode& n) 
  {
    if (n.tag == K_NOT) 
    {
      return (n.left) && ::check_cmp_op_for_transform(*n.left);
    } 
     
    return (((n.tag == K_EQ) || (n.tag == K_NEQ) || (n.tag == K_LT) || (n.tag == K_GT) || (n.tag == K_LEQ) || (n.tag == K_GEQ)) && 
      ((aq::util::is_column_reference(n.left)) && ((n.right) && ((n.right->tag == K_STRING) || (n.right->tag == K_INTEGER) || (n.right->tag == K_REAL))) ||
       (aq::util::is_column_reference(n.right)) && ((n.left) && ((n.left->tag == K_STRING) || (n.left->tag == K_INTEGER) || (n.left->tag == K_REAL)))));
  }

}

namespace aq {

class ExpressionTransform
{
public:
  ExpressionTransform(const Base& _baseDesc, const Settings& _settings) : baseDesc(_baseDesc), settings(_settings) {}
  template <typename M> aq::tnode * transform(aq::tnode * pNode);
private:
  enum transformation_type
  {
    CMP_OP_TRANS,
    BETWEEN_TRANS,
    LIKE_TRANS,
    NONE
  };
  template <typename T, class M> aq::tnode * transform(aq::tnode * pNode, transformation_type tt);
  template <typename T, typename M, class CMP> aq::tnode * transform(CMP& cmp);
  const Base& baseDesc;
  const Settings& settings;
  size_t tId;
  size_t cId;
  size_t cSize;
  aq::ColumnType cType;
};

template <typename T> 
class check_cmp_op
{
public:
  check_cmp_op(const aq::Base& _baseDesc, aq::tnode * node);
  void init();
  const aq::tnode * getColumnRef() const;
  bool check(const aq::ColumnItem<T>& item, const aq::ColumnType& cType) const;
  void success(aq::tnode * node);
private:
  const aq::Base& baseDesc;
  aq::ColumnItem<T> reference;
  aq::tnode * pNode;
  aq::tnode * pNodeColumnRef;
  aq::tnode * pNodeStr;
  aq::tnode * pNodeRes;
  bool	     bLeftColumnRef;
  short	     op_tag;
  bool reverseOp;
};

template <typename T> 
class check_between
{
public:
  check_between(const aq::Base& _baseDesc, aq::tnode * node);
  void init();
  const aq::tnode * getColumnRef() const;
  bool check(const aq::ColumnItem<T>& item, const aq::ColumnType& cType) const;
  void success(aq::tnode * node);
private:
  const aq::Base& baseDesc;
	aq::tnode * pNodeTmp;
	aq::tnode * pNodeColumnRef;
	aq::tnode * pNodeLeftBound;
	aq::tnode * pNodeRightBound;
	aq::tnode * pNodeRes;
  aq::ColumnItem<T> leftBound;
  aq::ColumnItem<T> rightBound;
  bool bNotBetween;
};

template <typename T> 
class check_like
{
public:
  check_like(const aq::Base& _baseDesc, aq::tnode * pNode);
  void init();
  const aq::tnode * getColumnRef() const;
  bool check(const aq::ColumnItem<T>& item, const aq::ColumnType& cType) const;
  void success(aq::tnode * node);
private:
  const aq::Base& baseDesc;
	aq::tnode * pNodeTmp;
	aq::tnode * pNodeColumnRef;
	aq::tnode * pNodeStr;
	aq::tnode * pNodeRes;
	ColumnType cType;
  bool bNotLike;
  
	// TPatternDescription	patternDesc;
  boost::regex rgx;
	mutable char szTmpBuf[100];
	mutable const char * pszVal;
	// int cEscape;
};

// ----------------------------------------------------------------------

template <typename T>
check_cmp_op<T>::check_cmp_op(const aq::Base& _baseDesc, aq::tnode * node) 
  : baseDesc(_baseDesc), pNode(node)
{
  this->init();
}

template <typename T>
void check_cmp_op<T>::init()
{
  reverseOp = false;
  if (pNode->tag == K_NOT)
  {
    pNode = pNode->left;
    reverseOp = !reverseOp;
  }

  if (aq::util::is_column_reference( pNode->left ) != 0) 
  {
    // Left is column reference -> check for strings at right 
    pNodeColumnRef = pNode->left;
    pNodeStr = pNode->right;
    bLeftColumnRef = true;
    op_tag = pNode->tag;
  } 
  else if (aq::util::is_column_reference(pNode->right) != 0) 
  {
    // Right is column reference -> check for strings at left
    pNodeColumnRef = pNode->right;
    pNodeStr = pNode->left;
    bLeftColumnRef = false;
    // Need to reverse the operations too !
    reverseOp = !reverseOp;
  }

  if (!pNodeColumnRef || !pNodeStr)
  {
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
  }

  if (aq::util::is_column_reference(pNodeColumnRef) == 0) 
  {
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
  }

  ::nodeToItem(*pNodeStr, reference);

  if (reverseOp)
  {
    if ( pNode->tag == K_LT )
      op_tag = K_GEQ;
    else if ( pNode->tag == K_GT ) 
      op_tag = K_LEQ;
    else if ( pNode->tag == K_LEQ ) 
      op_tag = K_GT;
    else if ( pNode->tag == K_GEQ )
      op_tag = K_LT;
  }
}

template <typename T>
const aq::tnode * check_cmp_op<T>::getColumnRef() const
{
  return this->pNodeColumnRef;
}

template <typename T>
bool check_cmp_op<T>::check(const aq::ColumnItem<T>& item, const aq::ColumnType& cType) const
{   
  bool rc = false;
  switch (op_tag)
  {
  case K_EQ:
    return !aq::ColumnItem<T>::lessThan(item, reference) && !aq::ColumnItem<T>::lessThan(reference, item);
    break;
  case K_NEQ:
    return aq::ColumnItem<T>::lessThan(item, reference) || aq::ColumnItem<T>::lessThan(reference, item);
    break;
  case K_LT:
    return aq::ColumnItem<T>::lessThan(item, reference);
    break;
  case K_GT:
    return aq::ColumnItem<T>::lessThan(reference, item);
    break;
  case K_LEQ:
    return !aq::ColumnItem<T>::lessThan(reference, item);
    break;
  case K_GEQ:
    return !aq::ColumnItem<T>::lessThan(item, reference);
    break;
  }
  assert(false);
  return false;
}

template <typename T>
void check_cmp_op<T>::success(aq::tnode * node)
{
  if (node->tag != K_FALSE) 
  {
    // Move the column reference subtree into the newly created subtree !
    node->left = this->pNodeColumnRef;	// which can be pNode->left or right !
    // Remove reference from the original subtree which will be deleted !
    if (this->bLeftColumnRef)
      this->pNode->left = nullptr;
    else
      this->pNode->right = nullptr;
  }
}

// ----------------------------------------------------------------------

template <typename T>
check_between<T>::check_between(const aq::Base& _baseDesc, aq::tnode * node)
  : baseDesc(_baseDesc), pNodeTmp(node)
{
  this->init();
}

template <typename T>
void check_between<T>::init()
{
  pNodeColumnRef = pNodeTmp->left;
  pNodeLeftBound = pNodeTmp->right->left;
  pNodeRightBound = pNodeTmp->right->right;
  pNodeRes = nullptr;
  bNotBetween = pNodeTmp->tag == K_NOT_BETWEEN;

  if (pNodeLeftBound->getDataType() != pNodeRightBound->getDataType())
  {
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
  }

  ::nodeToItem(*pNodeLeftBound, leftBound);
  ::nodeToItem(*pNodeRightBound, rightBound);
}

template <typename T>
const aq::tnode * check_between<T>::getColumnRef() const
{
  return this->pNodeColumnRef;
}

template <typename T>
bool check_between<T>::check(const aq::ColumnItem<T>& item, const aq::ColumnType& cType) const
{
  bool before = aq::ColumnItem<T>::lessThan(item, leftBound);
  bool after = aq::ColumnItem<T>::lessThan(rightBound, item);
  return ((!bNotBetween && !before && !after) || (bNotBetween && (before || after)));
}

template <typename T>
void check_between<T>::success(aq::tnode * node)
{
  if (node->tag != K_FALSE) 
  {
    // Move the column reference subtree into the newly created subtree !
    node->left = pNodeColumnRef;	// which is pNodeTmp->left; !
    // Remove reference from the original subtree which will be deleted !
    pNodeTmp->left = nullptr;
  }
}

// ----------------------------------------------------------------------

template <typename T>
check_like<T>::check_like(const aq::Base& _baseDesc, aq::tnode * pNode)
  : baseDesc(_baseDesc), pNodeTmp(pNode)
{
  this->init();
}

template <typename T>
void check_like<T>::init()
{
  pNodeColumnRef = pNodeTmp->left;
  pNodeStr = pNodeTmp->right;
  pNodeRes = nullptr;
  bNotLike = (pNodeTmp->tag == K_NOT_LIKE);

  if (pNodeStr == nullptr)
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");

  this->rgx = boost::regex(pNodeStr->getData().val_str);
}

template <typename T>
const aq::tnode * check_like<T>::getColumnRef() const
{
  return this->pNodeColumnRef;
}

template <typename T>
bool check_like<T>::check(const aq::ColumnItem<T>& item, const aq::ColumnType& cType) const
{
  szTmpBuf[0] = '\0';
  item.toString(szTmpBuf);
  pszVal = szTmpBuf;
  bool match = boost::regex_match(pszVal, this->rgx);
  return (match && !bNotLike) || (!match && bNotLike);
}

template <typename T>
void check_like<T>::success(aq::tnode * node)
{
  if (node->tag != K_FALSE) 
  {
    // Move the column reference subtree into the newly created subtree !
    node->left = pNodeColumnRef;	// which is pNodeTmp->left; !
    // Remove reference from the original subtree which will be deleted !
    pNodeTmp->left = nullptr;
  }
}

// ----------------------------------------------------------------------

template <typename M>
aq::tnode * ExpressionTransform::transform(aq::tnode * pNode)
{
  ::getColumnInfos(this->baseDesc, *pNode->left, this->tId, this->cId, this->cSize, this->cType);
  
  transformation_type tt = transformation_type::NONE;
  if (::check_between_for_transform(*pNode) != 0)
    tt = transformation_type::BETWEEN_TRANS;
  else if (::check_like_for_transform(*pNode) != 0) 
    tt = transformation_type::LIKE_TRANS;
  else if (::check_cmp_op_for_transform(*pNode) != 0)
    tt = transformation_type::CMP_OP_TRANS;
  
  switch (this->cType)
  {
  case aq::ColumnType::COL_TYPE_INT:
    pNode = this->transform<int32_t, M>(pNode, tt);
    break;
  case aq::ColumnType::COL_TYPE_BIG_INT:
  case aq::ColumnType::COL_TYPE_DATE:
    pNode = this->transform<int64_t, M>(pNode, tt);
    break;
  case aq::ColumnType::COL_TYPE_DOUBLE:
    pNode = this->transform<double, M>(pNode, tt);
    break;
  case aq::ColumnType::COL_TYPE_VARCHAR:
    pNode = this->transform<char*, M>(pNode, tt);
    break;
  }
	// delete_subtree(pNode); // FIXME

  //// Call recursively
  //if (pNode->next != nullptr)
  //{
  //  pNode->next = this->transform<M>(pNode->next);
  //}
  //if (pNode->left != nullptr)
  //{
  //  pNode->left = this->transform<M>(pNode->left);
  //}
  //// Do not call on K_PERIOD's node right branch if the right tag is K_COLUMN !
  //if ((pNode->right != nullptr) && ((pNode->tag != K_PERIOD) || (pNode->right->tag != K_COLUMN))) 
  //{
  //  pNode->right = this->transform<M>(pNode->right);
  //}

	return pNode;
}

template <typename T, typename M> 
aq::tnode * ExpressionTransform::transform(aq::tnode * pNode, transformation_type tt) 
{
  switch (tt)
  {
  case transformation_type::BETWEEN_TRANS:
    {
      check_between<T> cmp(this->baseDesc, pNode);
      return this->transform<T, M, check_between<T> >(cmp);
    }
    break;
  case transformation_type::LIKE_TRANS:
    {
      check_like<T> cmp(this->baseDesc, pNode);
      return this->transform<T, M, check_like<T> >(cmp);
    }
    break;
  case transformation_type::CMP_OP_TRANS:
    {
      check_cmp_op<T> cmp(this->baseDesc, pNode);
      return this->transform<T, M, check_cmp_op<T> >(cmp);
    }
    break;
  case transformation_type::NONE:
    break;
  }
  
  return pNode;
}

template <typename T>
struct column_item_cmp_t
{
  bool operator()(const aq::ColumnItem<T>& i1, const aq::ColumnItem<T>& i2) const
  {
    return aq::ColumnItem<T>::lessThan(i1, i2);
  }
};

template <typename T, typename M, class CMP>
aq::tnode * ExpressionTransform::transform(CMP& cmp) 
{
  aq::tnode * pNodeRes = nullptr;
  size_t index = 0;
  size_t matched = 0;
  T value;
  aq::ColumnItem<T> item;
  std::vector<aq::ColumnItem<T> > result, resultTmp1, resultTmp2, resultTmp3;
  column_item_cmp_t<T> column_cmp;
  boost::shared_ptr<aq::ColumnMapper_Intf<T> > cm(new aq::ThesaurusReader<T, M>(settings.dataPath.c_str(), tId, cId, cSize, settings.packSize));
  while (cm->loadValue(index++, &value) == 0)
  {
    item.setValue(value);
    if (cmp.check(item, cType)) 
    {
      ++matched;
      if (!resultTmp1.empty() && ColumnItem<T>::lessThan(item, *resultTmp1.rbegin()))
      {
        if (resultTmp2.empty())
        {
          std::merge(resultTmp1.begin(), resultTmp1.end(), resultTmp3.begin(), resultTmp3.end(), std::back_inserter(resultTmp2), column_cmp);
          resultTmp3.clear();
        }
        else
        {
          std::merge(resultTmp1.begin(), resultTmp1.end(), resultTmp2.begin(), resultTmp2.end(), std::back_inserter(resultTmp3), column_cmp);
          resultTmp2.clear();
        }
        result.clear();
        resultTmp1.clear();
        resultTmp1.push_back(item);
      }
      else
      {
        resultTmp1.push_back(item);
      }
    }
  }
  
  if (!resultTmp2.empty())
  {
    pNodeRes = ::getResult(resultTmp2, cType);
  }
  else if (!resultTmp3.empty())
  {
    pNodeRes = ::getResult(resultTmp3, cType);
  }
  else
  {
    std::merge(resultTmp1.begin(), resultTmp1.end(), resultTmp2.begin(), resultTmp2.end(), std::back_inserter(result), column_cmp);
    pNodeRes = ::getResult(result, cType);
  }
	if (pNodeRes == nullptr) 
  {
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
	}

  cmp.success(pNodeRes);

	return pNodeRes;
}

namespace expression_transform {
  
  template <class M>
  aq::tnode * transform(const aq::Base& base, const aq::Settings& settings, aq::tnode * node)
  {
    aq::tnode * newNode = node->clone_subtree();
    aq::ExpressionTransform expTr(base, settings);
    newNode = expTr.transform<M>(newNode);

    *node = *newNode;
    aq::tnode::delete_subtree(node->left);
    aq::tnode::delete_subtree(node->right);
    aq::tnode::delete_subtree(node->left);
    node->left = newNode->left;
    node->right = newNode->right;
    node->next = newNode->next;
    node->parent = nullptr;
    
    return node;
  }

}

}

#endif
