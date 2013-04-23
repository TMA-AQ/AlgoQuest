#ifndef __FIAN_EXPRTRANSFORM_H__
#define __FIAN_EXPRTRANSFORM_H__

/* For now only string (chars) values are supported ! */

#include "Column2Table.h"
#include "Table.h"

//------------------------------------------------------------------------------
tnode* expression_transform( tnode *pNode, Base* baseDesc, char* pszPath, int *pErr );

//------------------------------------------------------------------------------
int get_thesaurus_for_column_reference(	Column& thesaurus, tnode *pNode, int part,
										Base* baseDesc, char* pszPath, int *pErr );

#endif /* __FIAN_EXPRTRANSFORM_H__ */