#ifndef __AQ_EXPRTRANSFORM_H__
#define __AQ_EXPRTRANSFORM_H__

/* For now only string (chars) values are supported ! */

#include "Column2Table.h"
#include "Table.h"

//------------------------------------------------------------------------------
aq::tnode* expression_transform( aq::tnode *pNode, Base* baseDesc, char* pszPath, int *pErr );

//------------------------------------------------------------------------------
int get_thesaurus_for_column_reference(	Column& thesaurus, aq::tnode *pNode, int part,
										Base* baseDesc, char* pszPath, int *pErr );

#endif /* __AQ_EXPRTRANSFORM_H__ */