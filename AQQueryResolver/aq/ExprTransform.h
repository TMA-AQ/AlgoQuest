#ifndef __AQ_EXPRTRANSFORM_H__
#define __AQ_EXPRTRANSFORM_H__

#include "parser/SQLParser.h"
#include "parser/sql92_grm_tab.hpp"
#include "Column2Table.h"
#include "LIKE_PatternMatching.h"
#include "ColumnMapper_Intf.h"
#include "TreeUtilities.h"
#include "ThesaurusReader.h"
#include <aq/Exceptions.h>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

namespace 
{
  
  void nodeToItem(const aq::tnode& node, aq::ColumnItem& item)
  {

    switch (node.getDataType())
    {
    case aq::tnode::tnodeDataType::NODE_DATA_STRING:
      item.strval = node.getData().val_str;
      break;
    case aq::tnode::tnodeDataType::NODE_DATA_INT:
      item.numval = static_cast<double>(node.getData().val_int);
      break;
    case aq::tnode::tnodeDataType::NODE_DATA_NUMBER:
      item.numval = node.getData().val_number;
      break;
    }
  }
  
  aq::tnode * itemToNode(const aq::ColumnItem& item, const aq::ColumnType type)
  {
    aq::tnode * n = NULL;
    switch (type)
    {
    case aq::ColumnType::COL_TYPE_BIG_INT:
    case aq::ColumnType::COL_TYPE_INT: 
    case aq::ColumnType::COL_TYPE_DATE: 
      n = new aq::tnode( K_INTEGER );
      n->set_int_data((llong)item.numval );
      break;
    case aq::ColumnType::COL_TYPE_DOUBLE:
      n = new aq::tnode( K_REAL );
      n->set_double_data(item.numval );
      break;
    case aq::ColumnType::COL_TYPE_VARCHAR:
      n = new aq::tnode( K_STRING );
      n->set_string_data(item.strval.c_str() );
      break;
    }
    return n;
  }

  // template <typename... ARGS> a compiler with variadic template support will be nice ...
  template <class M>
  boost::shared_ptr<aq::ColumnMapper_Intf> getThesaurusReader(const aq::ColumnType& type, /*ARGS... args*/
    const char * path, size_t tId, size_t cId, size_t cSize, size_t packetSize)
  {
    boost::shared_ptr<aq::ColumnMapper_Intf> cm;
    switch (type)
    {
    case aq::ColumnType::COL_TYPE_DATE:
    case aq::ColumnType::COL_TYPE_BIG_INT:
      cm.reset(new aq::ThesaurusReader<int64_t, M>(path, tId, cId, cSize, packetSize));
      break;
    case aq::ColumnType::COL_TYPE_DOUBLE:
      cm.reset(new aq::ThesaurusReader<double, M>(path, tId, cId, cSize, packetSize));
      break;
    case aq::ColumnType::COL_TYPE_INT:
      cm.reset(new aq::ThesaurusReader<int32_t, M>(path, tId, cId, cSize, packetSize));
      break;
    case aq::ColumnType::COL_TYPE_VARCHAR:
      cm.reset(new aq::ThesaurusReader<char, M>(path, tId, cId, cSize, packetSize));
      break;
    }
    return cm;
  }

  aq::tnode* create_in_subtree(const std::vector<aq::ColumnItem>& items, const aq::ColumnType type, size_t nLevel = 0)
  {	
    aq::tnode *pNode = new aq::tnode(K_IN);

    if (items.empty())
      return NULL;

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

  aq::tnode* create_eq_subtree(const aq::ColumnItem& item, const aq::ColumnType type)
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
    assert(n != NULL);
    return n;
  }
  
  aq::tnode * getResult(const std::vector<aq::ColumnItem>& result, const aq::ColumnType& type)
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
      is_column_reference(n.left) && 
      n.right && (n.right->tag == K_AND) && 
      n.right->left && ((n.right->left->tag == K_STRING) || (n.right->left->tag == K_INTEGER)) && 
      n.right->right && ((n.right->right->tag == K_STRING) || (n.right->right->tag == K_INTEGER))); 
  }

  bool check_like_for_transform(const aq::tnode& n) 
  {
    return (((n.tag == K_LIKE) || (n.tag == K_NOT_LIKE)) &&
      (is_column_reference(n.left)) && 
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
      ((is_column_reference(n.left)) && ((n.right) && ((n.right->tag == K_STRING) || (n.right->tag == K_INTEGER) || (n.right->tag == K_REAL))) ||
       (is_column_reference(n.right)) && ((n.left) && ((n.left->tag == K_STRING) || (n.left->tag == K_INTEGER) || (n.left->tag == K_REAL)))));
  }

}

namespace aq {

class ExpressionTransform
{
public:
  ExpressionTransform(const Base& _baseDesc, const TProjectSettings& _settings);
  template <typename M> aq::tnode * transform(aq::tnode * pNode) const;
private:
  enum transformation_type
  {
    CMP_OP_TRANS,
    BETWEEN_TRANS,
    LIKE_TRANS,
    NONE
  };
  template <typename M, class CMP> aq::tnode * transform(aq::tnode * pNode, CMP& cmp) const;
  template <typename M> aq::tnode* transform_cmp_op(aq::tnode* pNode) const;
  template <typename M> aq::tnode* transform_like(aq::tnode* pNode) const;
  template <typename M> aq::tnode* transform_between(aq::tnode* pNode) const;
  const Base& baseDesc;
  const TProjectSettings& settings;
};

class check_cmp_op
{
public:
  check_cmp_op(const aq::Base& _baseDesc, aq::tnode * node);
  void init();
  const aq::tnode * getColumnRef() const;
  bool check(const aq::ColumnItem& item, const aq::ColumnType& cType) const;
  void success(aq::tnode * node);
private:
  const aq::Base& baseDesc;
  aq::ColumnItem reference;
  aq::tnode * pNode;
  aq::tnode * pNodeColumnRef;
  aq::tnode * pNodeStr;
  aq::tnode * pNodeRes;
  bool	     bLeftColumnRef;
  short	     op_tag;
  bool reverseOp;
};

class check_between
{
public:
  check_between(const aq::Base& _baseDesc, aq::tnode * node);
  void init();
  const aq::tnode * getColumnRef() const;
  bool check(const aq::ColumnItem& item, const aq::ColumnType& cType) const;
  void success(aq::tnode * node);
private:
  const aq::Base& baseDesc;
	aq::tnode * pNodeTmp;
	aq::tnode * pNodeColumnRef;
	aq::tnode * pNodeLeftBound;
	aq::tnode * pNodeRightBound;
	aq::tnode * pNodeRes;
  aq::ColumnItem leftBound;
  aq::ColumnItem rightBound;
  bool bNotBetween;
};

class check_like
{
public:
  check_like(const aq::Base& _baseDesc, aq::tnode * pNode);
  void init();
  const aq::tnode * getColumnRef() const;
  bool check(const aq::ColumnItem& item, const aq::ColumnType& cType) const;
  void success(aq::tnode * node);
private:
  const aq::Base& baseDesc;
	aq::tnode * pNodeTmp;
	aq::tnode * pNodeColumnRef;
	aq::tnode * pNodeStr;
	aq::tnode * pNodeRes;
	ColumnType cType;
  bool bNotLike;
  
	TPatternDescription	patternDesc;
	mutable char szTmpBuf[100];
	mutable const char * pszVal;
	int	cEscape;
};

template <typename M>
aq::tnode * ExpressionTransform::transform(aq::tnode * pNode) const
{
  transformation_type tt = transformation_type::NONE;
  if (::check_between_for_transform(*pNode) != 0)
    tt = transformation_type::BETWEEN_TRANS;
  else if (::check_like_for_transform(*pNode) != 0) 
    tt = transformation_type::LIKE_TRANS;
  else if (::check_cmp_op_for_transform(*pNode) != 0)
    tt = transformation_type::CMP_OP_TRANS;

  switch (tt)
  {
  case transformation_type::BETWEEN_TRANS:
    {
      check_between cmp(this->baseDesc, pNode);
      return this->transform<M>(pNode, cmp);
    }
    break;
  case transformation_type::LIKE_TRANS:
    {
      check_like cmp(this->baseDesc, pNode);
      return this->transform<M>(pNode, cmp);
    }
    break;
  case transformation_type::CMP_OP_TRANS:
    {
      check_cmp_op cmp(this->baseDesc, pNode);
      return this->transform<M>(pNode, cmp);
    }
    break;
  case transformation_type::NONE:
    break;
  }

	// Call recursively
  if (pNode->next != NULL)
  {
    pNode->next = this->transform<M>(pNode->next);
  }
  if (pNode->left != NULL)
  {
    pNode->left = this->transform<M>(pNode->left);
  }
  // Do not call on K_PERIOD's node right branch if the right tag is K_COLUMN !
  if ((pNode->right != NULL) && ((pNode->tag != K_PERIOD) || (pNode->right->tag != K_COLUMN))) 
  {
    pNode->right = this->transform<M>(pNode->right);
	}

	return pNode;
}

template <typename M, class CMP> 
aq::tnode * ExpressionTransform::transform(aq::tnode * pNode, CMP& cmp) const
{
  aq::tnode * pNodeRes = NULL;
  size_t index = 0;
  std::vector<aq::ColumnItem> result;
  aq::ColumnItem item;
  
  cmp.init();

  size_t tId, cId, cSize;
  aq::ColumnType cType;
  ::getColumnInfos(this->baseDesc, *cmp.getColumnRef(), tId, cId, cSize, cType);

  size_t matched = 0;
  boost::shared_ptr<aq::ColumnMapper_Intf> cm = ::getThesaurusReader<M>(cType, settings.dataPath.c_str(), tId, cId, cSize, settings.packSize);
  std::vector<aq::ColumnItem> resultTmp1, resultTmp2, resultTmp3;
  aq::column_cmp_t column_cmp;
  while (cm->loadValue(index++, item) == 0)
  {
    if (cmp.check(item, cType)) 
    {
      ++matched;
      if (!resultTmp1.empty() && ColumnItem::lessThan(&item, &*resultTmp1.rbegin(), cType))
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
	if (pNodeRes == NULL) 
  {
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
	}

  cmp.success(pNodeRes);

	delete_subtree(pNode);
	return pNodeRes;
}

namespace expression_transform {
  
  template <class M>
  aq::tnode * transform(const aq::Base& base, const aq::TProjectSettings& settings, aq::tnode * node)
  {
    aq::tnode * newNode = aq::clone_subtree(node);
    aq::ExpressionTransform expTr(base, settings);
    newNode = expTr.transform<M>(newNode);

    *node = *newNode;
    aq::delete_subtree(node->left);
    aq::delete_subtree(node->right);
    aq::delete_subtree(node->next);
    node->left = newNode->left;
    node->right = newNode->right;
    node->next = newNode->next;
    node->parent = NULL;
    
    return node;
  }

}

}

#endif
