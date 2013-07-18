#include "ExprTransform.h"

namespace aq
{
  
ExpressionTransform::ExpressionTransform(const Base& _baseDesc, const TProjectSettings& _settings)
  : baseDesc(_baseDesc), settings(_settings)
{
}

// ----------------------------------------------------------------------

check_cmp_op::check_cmp_op(const aq::Base& _baseDesc, aq::tnode * node) 
  : baseDesc(_baseDesc), pNode(node)
{
}

void check_cmp_op::init()
{
  reverseOp = false;
  if (pNode->tag == K_NOT)
  {
    pNode = pNode->left;
    reverseOp = !reverseOp;
  }

  if (is_column_reference( pNode->left ) != 0) 
  {
    // Left is column reference -> check for strings at right 
    pNodeColumnRef = pNode->left;
    pNodeStr = pNode->right;
    bLeftColumnRef = true;
    op_tag = pNode->tag;
  } 
  else if (is_column_reference(pNode->right) != 0) 
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

  if (is_column_reference(pNodeColumnRef) == 0) 
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

const aq::tnode * check_cmp_op::getColumnRef() const
{
  return this->pNodeColumnRef;
}

bool check_cmp_op::check(const aq::ColumnItem& item, const aq::ColumnType& cType) const
{    
  switch (op_tag)
  {
  case K_LT:
    return aq::ColumnItem::lessThan(&item, &reference, cType);
    break;
  case K_GT:
    return aq::ColumnItem::lessThan(&reference, &item, cType);
    break;
  case K_LEQ:
    return !aq::ColumnItem::lessThan(&reference, &item, cType);
    break;
  case K_GEQ:
    return !aq::ColumnItem::lessThan(&item, &reference, cType);
    break;
  }
  assert(false);
  return false;
}

void check_cmp_op::sucess(aq::tnode * node)
{
  if (node->tag != K_FALSE) 
  {
    // Move the column reference subtree into the newly created subtree !
    node->left = this->pNodeColumnRef;	// which can be pNode->left or right !
    // Remove reference from the original subtree which will be deleted !
    if (this->bLeftColumnRef)
      this->pNode->left = NULL;
    else
      this->pNode->right = NULL;
  }
}

// ----------------------------------------------------------------------

check_between::check_between(const aq::Base& _baseDesc, aq::tnode * node)
  : baseDesc(_baseDesc), pNodeTmp(node)
{
}

void check_between::init()
{
  pNodeColumnRef = pNodeTmp->left;
  pNodeLeftBound = pNodeTmp->right->left;
  pNodeRightBound = pNodeTmp->right->right;
  pNodeRes = NULL;
  bNotBetween = pNodeTmp->tag == K_NOT_BETWEEN;

  if (pNodeLeftBound->getDataType() != pNodeRightBound->getDataType())
  {
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
  }

  ::nodeToItem(*pNodeLeftBound, leftBound);
  ::nodeToItem(*pNodeRightBound, rightBound);
}

const aq::tnode * check_between::getColumnRef() const
{
  return this->pNodeColumnRef;
}

bool check_between::check(const aq::ColumnItem& item, const aq::ColumnType& cType) const
{
  bool before = aq::ColumnItem::lessThan(&item, &leftBound, cType);
  bool after = aq::ColumnItem::lessThan(&rightBound, &item, cType);
  return ((!bNotBetween && !before && !after) || (bNotBetween && (before || after)));
}

void check_between::sucess(aq::tnode * node)
{
  if (node->tag != K_FALSE) 
  {
    // Move the column reference subtree into the newly created subtree !
    node->left = pNodeColumnRef;	// which is pNodeTmp->left; !
    // Remove reference from the original subtree which will be deleted !
    pNodeTmp->left = NULL;
  }
}

// ----------------------------------------------------------------------

check_like::check_like(const aq::Base& _baseDesc, aq::tnode * pNode)
  : baseDesc(_baseDesc), pNodeTmp(pNode)
{
}

void check_like::init()
{
  pNodeColumnRef = pNodeTmp->left;
  pNodeStr = pNodeTmp->right;
  pNodeRes = NULL;
  bNotLike = (pNodeTmp->tag == K_NOT_LIKE);

  cEscape = NO_ESCAPE_CHAR;
  if (pNodeStr != NULL && pNodeStr->tag == K_ESCAPE) 
  {
    if (pNodeStr->right != NULL && pNodeStr->right->getData().val_str != NULL)
      cEscape = pNodeStr->right->getData().val_str[ 0 ];
    if (cEscape == '\0')
      cEscape = NO_ESCAPE_CHAR;
    pNodeStr = pNodeStr->left;
  }
  if (pNodeStr == NULL)
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");

  if (PatternMatchingCreate( pNodeStr->getData().val_str, cEscape, &patternDesc) == -1 ) 
  {
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
  }
}

const aq::tnode * check_like::getColumnRef() const
{
  return this->pNodeColumnRef;
}

bool check_like::check(const aq::ColumnItem& item, const aq::ColumnType& cType) const
{
  szTmpBuf[0] = '\0';
  item.toString(szTmpBuf, cType);
  pszVal = szTmpBuf;
  int nRet = MatchPattern(pszVal, patternDesc);
  return ((!bNotLike && (nRet == 1)) || (bNotLike && (nRet == 0)));
}

void check_like::sucess(aq::tnode * node)
{
  if (node->tag != K_FALSE) 
  {
    // Move the column reference subtree into the newly created subtree !
    node->left = pNodeColumnRef;	// which is pNodeTmp->left; !
    // Remove reference from the original subtree which will be deleted !
    pNodeTmp->left = NULL;
  }
}

}