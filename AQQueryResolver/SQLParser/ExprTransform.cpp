#include "ExprTransform.h"
#include "SQLParser.h"
#include "sql92_grm_tab.h"
#include "Column2Table.h"
#include "LIKE_PatternMatching.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>						/* _MAX_PATH */
#include <aq/Utilities.h>
#include <boost/bind.hpp>

using namespace aq;

//------------------------------------------------------------------------------
// PRIVATE

//------------------------------------------------------------------------------
#ifndef _MAX_PATH
#define _MAX_PATH 256
#endif 

//------------------------------------------------------------------------------
/* Return 1 for true, 0 for false */
int is_column_reference( tnode *pNode ) {
	if ( pNode == NULL )
		return 0;
	if ( pNode->tag != K_PERIOD )
		return 0;
	if ( pNode->left == NULL )
		return 0;
	if ( pNode->right == NULL )
		return 0;
	if ( pNode->left->tag != K_IDENT )
		return 0;
	if ( pNode->right->tag != K_COLUMN )
		return 0;
	return 1;
}

//------------------------------------------------------------------------------
int get_thesaurus_for_column(	Column& thesaurus, 
								unsigned int nTableId, unsigned int nColumnId, 
								unsigned int nColumnSize,
								unsigned int nPartNumber, ColumnType eColumnType, 
								char* pszPath, int *pErr ) {

	 /* Try Binary File First ! */
	std::string fileName = getThesaurusFileName( pszPath, nTableId, nColumnId, nPartNumber );
	 
	 if ( thesaurus.loadFromThesaurus( fileName.c_str(), 0, nColumnSize, 
			eColumnType, pErr ) != 0 ) {
		 /* Try Text Version Next ! */
		 fileName += ".txt";
		 if ( thesaurus.loadFromThesaurus( fileName.c_str(), 1, nColumnSize, 
				eColumnType, pErr ) != 0 ) {
			 /* File not found or Error Loading thesaurus file ! */
			 return -1;
		 }
	 }

	 return 0;
}

//------------------------------------------------------------------------------
/* Return -1 on error ! */
int get_thesaurus_info_for_column_reference( tnode *pNode, Base* baseDesc, 
											unsigned int *pnTableId, unsigned int *pnColumnId, 
											unsigned int *pnColumnSize, 
											ColumnType *peColumnType, int *pErr ) {

												if ( is_column_reference( pNode ) == 0 ) {
													if ( pErr != NULL )
														*pErr = EXPR_TR_ERR_NOT_COLUMN_REFERENCE;
													return -1;
												}

												if ( get_table_and_column_id_from_table_array(	baseDesc, pNode->left->data.val_str /*tbl_name*/,
													pNode->right->data.val_str /*column_name*/, 
													pnTableId, pnColumnId, pnColumnSize,
													peColumnType ) == -1 ) {
														if ( pErr != NULL )
															*pErr = EXPR_TR_ERR_TBL_OR_COL_ID_NOT_FOUND;
														return -1;
												}

												return 0;
}

//------------------------------------------------------------------------------
/* nLevel = 0 we have the K_IN node */
/* nLevel = 1 we have K_COMMA->left=value, ->right=K_COMMA, ... */
tnode* create_in_subtree( unsigned int nLevel, Column& thesaurus ) {
	tnode *pNode;
	tnode *pNodeRight;

	pNode		= NULL;
	pNodeRight	= NULL;

	if ( thesaurus.Items.size() == 0 )
		return NULL;

	if ( nLevel < thesaurus.Items.size() - 1 ) {
		/* More then one element ! */
		pNodeRight = create_in_subtree( nLevel + 1, thesaurus );
		if ( pNodeRight == NULL )
			return NULL;
		pNode = new_node( K_COMMA );
		if ( pNode == NULL ) {
			delete_subtree( pNodeRight );
			return NULL;
		}

		if ( thesaurus.Type == COL_TYPE_INT || thesaurus.Type == COL_TYPE_DATE1
			 || thesaurus.Type == COL_TYPE_DATE2 || thesaurus.Type == COL_TYPE_DATE3 ) {
			pNode->left = new_node( K_INTEGER );
			if ( set_int_data( pNode->left, (llong) thesaurus.Items[ nLevel ]->numval ) == NULL ) {
				delete_node( pNode->left );
				delete_subtree( pNodeRight );
				return NULL;
			}
		} else if ( thesaurus.Type == COL_TYPE_DOUBLE ) {
			pNode->left = new_node( K_REAL );
			if ( set_double_data( pNode->left, thesaurus.Items[ nLevel ]->numval ) == NULL ) {
				delete_node( pNode->left );
				delete_subtree( pNodeRight );
				return NULL;
			}
		} else {	// if ( thesaurus.Type == COL_TYPE_VARCHAR )
			pNode->left = new_node( K_STRING );
			if ( set_string_data( pNode->left, thesaurus.Items[ nLevel ]->strval.c_str() ) == NULL ) {
				delete_node( pNode->left );
				delete_subtree( pNodeRight );
				return NULL;
			}
		}

		pNode->right = pNodeRight;
		pNodeRight	 = pNode; 
		pNode		 = NULL;
	} else if ( nLevel == thesaurus.Items.size() - 1 ) {
		/* One element */
		if ( thesaurus.Type == COL_TYPE_INT || thesaurus.Type == COL_TYPE_DATE1
			|| thesaurus.Type == COL_TYPE_DATE2 || thesaurus.Type == COL_TYPE_DATE3) {
			pNodeRight = new_node( K_INTEGER );
			if ( set_int_data( pNodeRight, (llong) thesaurus.Items[ nLevel ]->numval ) == NULL ) {
				delete_node( pNodeRight );
				return NULL;
			}
		} else if ( thesaurus.Type == COL_TYPE_DOUBLE ) {
			pNodeRight = new_node( K_REAL );
			if ( set_double_data( pNodeRight, thesaurus.Items[ nLevel ]->numval ) == NULL ) {
				delete_node( pNodeRight );
				return NULL;
			}
		} else {	// if ( thesaurus.Type == COL_TYPE_VARCHAR )
			pNodeRight = new_node( K_STRING );
			if ( set_string_data( pNodeRight, thesaurus.Items[ nLevel ]->strval.c_str() ) == NULL ) {
				delete_node( pNodeRight );
				return NULL;
			}
		}
	}

	if ( nLevel == 0 ) {
		/* Add the K_IN node too ! */
		pNode = new_node( K_IN );
		if ( pNode == NULL ) {
			delete_node( pNodeRight );
			return NULL;
		}
		pNode->right = pNodeRight;
	} else 
		pNode = pNodeRight;

	return pNode;
}

//------------------------------------------------------------------------------
tnode* create_eq_subtree_string( const char* pszRightItem ) {
	tnode *pNode;

	pNode = new_node( K_EQ );
	if ( pNode == NULL )
		return NULL;

	pNode->right = new_node( K_STRING );
	if ( set_string_data( pNode->right, pszRightItem ) == NULL ) {
		delete_subtree( pNode );
		return NULL;
	}

	return pNode;
}

//------------------------------------------------------------------------------
tnode* create_eq_subtree_integer( llong nRightItem ) {
	tnode *pNode;

	pNode = new_node( K_EQ );
	if ( pNode == NULL )
		return NULL;

	pNode->right = new_node( K_INTEGER );
	if ( set_int_data( pNode->right, nRightItem ) == NULL ) {
		delete_subtree( pNode );
		return NULL;
	}

	return pNode;
}

//------------------------------------------------------------------------------
tnode* create_eq_subtree_double( double dRightItem ) {
	tnode *pNode;

	pNode = new_node( K_EQ );
	if ( pNode == NULL )
		return NULL;

	pNode->right = new_node( K_REAL );
	if ( set_double_data( pNode->right, dRightItem ) == NULL ) {
		delete_subtree( pNode );
		return NULL;
	}

	return pNode;
}


//------------------------------------------------------------------------------
/* Return NULL on error, or a new allocated string with malloc ! */
char* get_string_without_quotes( char* pszStr ) {
	char *pszTmp;
	size_t nLen;

	if ( pszStr == NULL )
		return NULL;

	pszTmp = (char*)malloc( strlen( pszStr ) + 1 );
	if ( pszTmp == NULL )
		return NULL;

	if ( pszStr[ 0 ] == '\'' )
		strcpy( pszTmp, pszStr + 1 );
	else
		strcpy( pszTmp, pszStr );

	nLen = strlen( pszTmp );
	if ( nLen > 0 ) {
		if ( pszTmp[ nLen - 1 ] == '\'' )
			pszTmp[ nLen - 1 ] = '\0';
	}

	return pszTmp;
}

//------------------------------------------------------------------------------
/* Return 1 on true, 0 on false */
int check_between_for_transform( tnode *pNode ) {
	if ( pNode == NULL )
		return 0;

	if ( pNode->tag == K_BETWEEN || pNode->tag == K_NOT_BETWEEN ) {
		/* BETWEEN */
		if ( is_column_reference( pNode->left ) != 0 ) {
			/* Left is column reference -> check for strings at right */
			tnode *pNodeTmp;
			pNodeTmp = pNode->right;
			if ( pNodeTmp != NULL && pNodeTmp->tag == K_AND ) {
				if (	pNodeTmp->left != NULL && 
					( pNodeTmp->left->tag == K_STRING || pNodeTmp->left->tag == K_INTEGER ) &&
					pNodeTmp->right != NULL && 
					( pNodeTmp->right->tag == K_STRING || pNodeTmp->right->tag == K_INTEGER ) ) {
						return 1;
				}
			}
		}
	}
	return 0;
}

//------------------------------------------------------------------------------
typedef enum {
	BCS_Before_Left_Bound,					// ... < LeftBound
	BCS_Between_Left_and_Right_Bound,		// LeftBound <= ... <= RightBound
	BCS_After_Right_Bound					// RightBound < ...
} TBoundComparisonState;

//------------------------------------------------------------------------------
/* NOT BETWEEN :
*	pNode						=> K_NOT
*	pNode->left					=> K_BETWEEN
*	pNode->left->left			=> column_reference
*	pNode->left->right			=> K_AND
*	pNode->left->right->left	=> K_STRING
*	pNode->left->right->right	=> K_STRING
* BETWEEN :
*	pNode						=> K_BETWEEN
*	pNode->left					=> column_reference
*	pNode->right				=> K_AND
*	pNode->right->left			=> K_STRING
*	pNode->right->right			=> K_STRING
*/
//------------------------------------------------------------------------------
tnode* transform_between( tnode* pNode, Base* baseDesc, char* pszPath, int *pErr ) {
	int			bNotBetween;
	tnode		*pNodeTmp;
	tnode		*pNodeColumnRef;
	tnode		*pNodeStrBoundLeft;
	tnode		*pNodeStrBoundRight;
	tnode		*pNodeRes;
	Column		thesaurus;
	Column		thesaurusRes;
	int			bAddValue;
	TBoundComparisonState	eState;	
	unsigned int			i;
	unsigned int	nTableId;
	unsigned int	nColumnId;
	unsigned int	nColumnSize;
	ColumnType		eColumnType;
	unsigned int	nLoopCnt;

	if ( pNode->tag == K_NOT_BETWEEN )
		bNotBetween = 1;
	else
		bNotBetween = 0;

	pNodeTmp = pNode;

	pNodeColumnRef		= pNodeTmp->left;
	pNodeStrBoundLeft	= pNodeTmp->right->left;
	pNodeStrBoundRight	= pNodeTmp->right->right;

	if ( get_thesaurus_info_for_column_reference( pNodeColumnRef, baseDesc, &nTableId, &nColumnId, &nColumnSize, &eColumnType, pErr ) == -1 )
		return pNode;

	/* Keep the ColumnType ! */
	thesaurusRes.Type = eColumnType;
	if( thesaurusRes.Type == COL_TYPE_BIG_INT ) //debug13 - quick fix
		thesaurusRes.Type = COL_TYPE_INT;

	/* Loop on Thesaurus Parts (biggest nLoopCnt is 999, three digit !) */
	for ( nLoopCnt = 0; nLoopCnt < 1000; nLoopCnt++ ) {

		if ( get_thesaurus_for_column( thesaurus, nTableId, nColumnId, nColumnSize, 
			nLoopCnt, eColumnType, pszPath, pErr ) != 0 ) {
			if ( nLoopCnt != 0 && *pErr == EXPR_TR_ERR_THESAURUS_FILE_NOT_FOUND ) {
				*pErr = 0;
				break;	/* No more Parts ! */
			}

			return pNode;	/* Keep old node */
		}

		/* for NOT add the values which are not in the interval, otherwise add those from interval */
		bAddValue = bNotBetween;
		eState = BCS_Before_Left_Bound;
		for ( i = 0; i < thesaurus.Items.size(); i++ ) {
			if ( eState == BCS_Before_Left_Bound ) {
				/* Compare with LeftBound - toggle if value is >= */ 
				if ( thesaurus.Type == COL_TYPE_INT || thesaurus.Type == COL_TYPE_DOUBLE ||
					(thesaurus.Type >= COL_TYPE_DATE1 && thesaurus.Type <= COL_TYPE_DATE4 )
					) {
					if ( pNodeStrBoundLeft->tag == K_REAL && thesaurus.Items[ i ]->numval >= pNodeStrBoundLeft->data.val_number
						|| pNodeStrBoundLeft->tag == K_INTEGER && thesaurus.Items[ i ]->numval >= pNodeStrBoundLeft->data.val_int ){
						eState = BCS_Between_Left_and_Right_Bound;
						bAddValue = ( bAddValue == 0 ) ? 1 : 0;
					}
				} else { // if ( thesaurus.Type == COL_TYPE_VARCHAR )
					if ( strcmp( thesaurus.Items[ i ]->strval.c_str(), pNodeStrBoundLeft->data.val_str ) >= 0 ) {
						eState = BCS_Between_Left_and_Right_Bound;
						bAddValue = ( bAddValue == 0 ) ? 1 : 0;
					}
				}
			}

			if ( eState == BCS_Between_Left_and_Right_Bound ) {
				/* Compare with RightBound - toggle if value is > */ 
				if ( thesaurus.Type == COL_TYPE_INT || thesaurus.Type == COL_TYPE_DOUBLE ||
					(thesaurus.Type >= COL_TYPE_DATE1 && thesaurus.Type <= COL_TYPE_DATE4 )
					) {
					if ( pNodeStrBoundRight->tag == K_REAL && thesaurus.Items[ i ]->numval > pNodeStrBoundRight->data.val_number
						|| pNodeStrBoundRight->tag == K_INTEGER && thesaurus.Items[ i ]->numval > pNodeStrBoundRight->data.val_int ){
						eState = BCS_After_Right_Bound;
						bAddValue = ( bAddValue == 0 ) ? 1 : 0;
					}
				} else { // if ( thesaurus.Type == COL_TYPE_VARCHAR )
					if ( strcmp( thesaurus.Items[ i ]->strval.c_str(), pNodeStrBoundRight->data.val_str ) > 0 ) {
						eState = BCS_After_Right_Bound;
						bAddValue = ( bAddValue == 0 ) ? 1 : 0;
					}
				}
			}

			if ( eState == BCS_After_Right_Bound && bAddValue == 0 )
				break; /* Nothing more to do ! */

			if ( bAddValue == 1 ) {
					thesaurusRes.Items.push_back( thesaurus.Items[ i ] );
			}
		}

	} /* & Loop on Thesaurus Parts */

	if ( thesaurusRes.Items.size() == 0 ) {
		/* No Match -> Expression evaluates to FALSE ! */
		pNodeRes = new_node( K_FALSE );
	} else if ( thesaurusRes.Items.size() == 1 ) {
		/* Replace it with operator = */
		if ( thesaurusRes.Type == COL_TYPE_INT ) {
			pNodeRes = create_eq_subtree_integer( (llong) thesaurusRes.Items[ 0 ]->numval );
		} else { // if ( thesaurusRes.Type == COL_TYPE_VARCHAR )
			pNodeRes = create_eq_subtree_string( thesaurusRes.Items[ 0 ]->strval.c_str() );
		}
	} else {
		/* Replace it with operator IN */
		pNodeRes = create_in_subtree( 0, thesaurusRes );
	}

	if ( pNodeRes == NULL ) {
		/* Report Error ! */
		if ( pErr != NULL )
			*pErr = EXPR_TR_ERR_NOT_ENOUGH_MEMORY;
		return pNode;	/* Keep old node */
	}

	/* Success */
	if ( pNodeRes->tag != K_FALSE ) {
		/* Move the column reference subtree into the newly created subtree ! */
		pNodeRes->left = pNodeColumnRef;	/* which is pNodeTmp->left; ! */
		/* Remove reference from the original subtree which will be deleted ! */
		pNodeTmp->left = NULL;
	}

	/* Free pNode */
	delete_subtree( pNode );
	return pNodeRes;
}

//------------------------------------------------------------------------------
/* Return 1 on true, 0 on false */
int check_like_for_transform( tnode *pNode ) {
	if ( pNode == NULL )
		return 0;

	if ( pNode->tag == K_LIKE || pNode->tag == K_NOT_LIKE ) {
		/* LIKE */
		if ( is_column_reference( pNode->left ) != 0 ) {
			/* Left is column reference -> check for string or ESCAPE right */
			tnode *pNodeTmp;
			pNodeTmp = pNode->right;
			if ( pNodeTmp != NULL ) {
				if ( pNodeTmp->tag == K_STRING )
					return 1;
				if ( pNodeTmp->tag == K_ESCAPE ) {
					if (	pNodeTmp->left != NULL && pNodeTmp->left->tag == K_STRING &&
						pNodeTmp->right != NULL && pNodeTmp->right->tag == K_STRING )
						return 1;
				}
			}
		}
	}
	return 0;
}

//------------------------------------------------------------------------------
/* NOT LIKE :
*	pNode						=> K_NOT
*	pNode->left					=> K_LIKE
*	pNode->left->left			=> column_reference
*	pNode->left->right			=> K_STRING
* OR :
*	pNode						=> K_NOT
*	pNode->left					=> K_LIKE
*	pNode->left->left			=> column_reference
*	pNode->left->right			=> K_ESCAPE
*	pNode->left->right->left	=> K_STRING ( Pattern )
*	pNode->left->right->right	=> K_STRING ( Escape char )
* LIKE :
*	pNode						=> K_LIKE
*	pNode->left					=> column_reference
*	pNode->right				=> K_STRING
* OR :
*	pNode						=> K_LIKE
*	pNode->left					=> column_reference
*	pNode->right				=> K_ESCAPE
*	pNode->right->left			=> K_STRING ( Pattern )
*	pNode->right->right			=> K_STRING ( Escape char )
*/
//------------------------------------------------------------------------------
tnode* transform_like( tnode* pNode, Base* baseDesc, char* pszPath, int *pErr ) {
	int			bNotLike;
	tnode		*pNodeTmp;
	tnode		*pNodeColumnRef;
	tnode		*pNodeStr;
	tnode		*pNodeRes;
	Column		thesaurus;
	Column		thesaurusRes;
	int				nRet;
	unsigned int	i;
	unsigned int	nTableId;
	unsigned int	nColumnId;
	unsigned int	nColumnSize;
	ColumnType		eColumnType;
	unsigned int	nLoopCnt;
	TPatternDescription	patternDesc;
	char			szTmpBuf[ 100 ];
	const char		*pszVal;
	int				cEscape = NO_ESCAPE_CHAR;

	if ( pNode->tag == K_NOT_LIKE )
		bNotLike = 1;
	else
		bNotLike = 0;
	pNodeTmp = pNode;

	pNodeColumnRef	= pNodeTmp->left;
	pNodeStr		= pNodeTmp->right;

	if ( pNodeStr != NULL && pNodeStr->tag == K_ESCAPE ) {
		if ( pNodeStr->right != NULL && pNodeStr->right->data.val_str != NULL )
			cEscape = pNodeStr->right->data.val_str[ 0 ];
		if ( cEscape == '\0' )
			cEscape = NO_ESCAPE_CHAR;
		pNodeStr = pNodeStr->left;
	}
	if ( pNodeStr == NULL )
		return pNode;

	if ( get_thesaurus_info_for_column_reference( pNodeColumnRef, baseDesc, &nTableId, &nColumnId, &nColumnSize, &eColumnType, pErr ) == -1 )
		return pNode;

	/* Prepare PatterMatching */
	//!!!!	
	/* USE cEscape ! */
	if ( PatternMatchingCreate( pNodeStr->data.val_str, cEscape, &patternDesc ) == -1 ) {
		if ( pErr != NULL )
			*pErr = EXPR_TR_ERR_PREPARING_PATTERN_MATCHING;
		return pNode;
	}

	/* Using column's thesaurus create a new one with those values which fulfills the condition ! */
	/* Keep the ColumnType ! */
	thesaurusRes.Type = eColumnType;
	if( thesaurusRes.Type == COL_TYPE_BIG_INT ) //debug13 - quick fix
		thesaurusRes.Type = COL_TYPE_INT;

	/* Loop on Thesaurus Parts (bigest nLoopCnt is 999, three digit !) */
	for ( nLoopCnt = 0; nLoopCnt < 1000; nLoopCnt++ ) {

		if ( get_thesaurus_for_column( thesaurus, nTableId, nColumnId, nColumnSize,
				nLoopCnt, eColumnType, pszPath, pErr ) != 0 ) {
			if ( nLoopCnt != 0 && *pErr == EXPR_TR_ERR_THESAURUS_FILE_NOT_FOUND ) {
				*pErr = 0;
				break;	/* No more Parts ! */
			}

			/* Destroy Pattern Matching */
			PatternMatchingDestroy( &patternDesc );
			/* Delete the temporary thesaurus */
			return pNode;	/* Keep old node */
		}

		/* for NOT add the values which doesn't Match, otherwise add those that Match */
		for ( i = 0; i < thesaurus.Items.size(); i++ ) {
			/* We don't have COL_TYPE_INT type because we forced to be loaded as strings ! */
			if ( thesaurus.Type == COL_TYPE_INT ) {
				szTmpBuf[ 0 ] = '\0';
				sprintf( szTmpBuf, "%d", thesaurus.Items[ i ]->numval );
				pszVal = szTmpBuf;
			} else { // if ( thesaurus.Type == COL_TYPE_VARCHAR )
				pszVal = thesaurus.Items[ i ]->strval.c_str();
			}

			nRet = MatchPattern( pszVal, patternDesc );
			if ( nRet == -1 ) {
				if ( pErr != NULL )
					*pErr = EXPR_TR_ERR_PATTERN_MATCHING;
				PatternMatchingDestroy( &patternDesc );
				return pNode;
			}

			if ( ( bNotLike == 0 && nRet == 1 ) || ( bNotLike == 1 && nRet == 0 ) ) {
				thesaurusRes.Items.push_back( thesaurus.Items[ i ] );
			}
		}

	} /* & Loop on Thesaurus Parts */

	/* Destroy Pattern Matching */
	PatternMatchingDestroy( &patternDesc );

	if ( thesaurusRes.Items.size() == 0 ) {
		/* No Match -> Expression evaluates to FALSE ! */
		pNodeRes = new_node( K_FALSE );
	} else if ( thesaurusRes.Items.size() == 1 ) {
		/* Replace it with operator = */
		if ( thesaurusRes.Type == COL_TYPE_INT ) {
			pNodeRes = create_eq_subtree_integer( (llong) thesaurusRes.Items[ 0 ]->numval );
		} else { // if ( thesaurusRes.Type == COL_TYPE_VARCHAR )
			pNodeRes = create_eq_subtree_string( thesaurusRes.Items[ 0 ]->strval.c_str() );
		}
	} else {
		/* Replace it with operator IN */
		pNodeRes = create_in_subtree( 0, thesaurusRes );
	}

	if ( pNodeRes == NULL ) {
		/* Report Error ! */
		if ( pErr != NULL )
			*pErr = EXPR_TR_ERR_NOT_ENOUGH_MEMORY;
		return pNode;	/* Keep old node */
	}

	/* Success */
	if ( pNodeRes->tag != K_FALSE ) {
		/* Move the column reference subtree into the newly created subtree ! */
		pNodeRes->left = pNodeColumnRef;	/* which is pNodeTmp->left; ! */
		/* Remove reference from the original subtree which will be deleted ! */
		pNodeTmp->left = NULL;
	}

	/* Free pNode */
	delete_subtree( pNode );
	return pNodeRes;
}

//------------------------------------------------------------------------------
/* Return 1 on true, 0 on false */
int check_cmp_op_for_transform( tnode *pNode ) {
	if ( pNode == NULL )
		return 0;

	if ( pNode->tag == K_NOT ) {
		/* Check for NOT <, >, <=, >= */
		return check_cmp_op_for_transform( pNode->left );
	} else
	{
		/* Check for <, >, <=, >= */
		if ( pNode->tag == K_LT || pNode->tag == K_GT || pNode->tag == K_LEQ || pNode->tag == K_GEQ ) {
			if ( is_column_reference( pNode->left ) != 0 ) {
				/* Left is column reference -> check for strings at right */
				if (	pNode->right != NULL && 
					( pNode->right->tag == K_STRING || pNode->right->tag == K_INTEGER || pNode->right->tag == K_REAL ) )
					return 1;
			} else if ( is_column_reference( pNode->right ) != 0 ) {
				/* Right is column reference -> check for strings at left */
				if (	pNode->left != NULL && 
					( pNode->left->tag == K_STRING || pNode->left->tag == K_INTEGER || pNode->left->tag == K_REAL ) )
					return 1;
			}
		}
	}
	return 0;
}

//------------------------------------------------------------------------------
/* cmp_op :
*	pNode						=> K_LT or K_GT or K_LEQ or K_GEQ
*	pNode->left					=> column_reference
*	pNode->right				=> K_STRING
* OR : 
*	pNode						=> K_LT or K_GT or K_LEQ or K_GEQ
*	pNode->left					=> K_STRING
*	pNode->right				=> column_reference
*/
//------------------------------------------------------------------------------
tnode* transform_cmp_op( tnode* pNode, Base* baseDesc, char* pszPath, int *pErr ) {
	tnode		*pNodeColumnRef;
	tnode		*pNodeStr;
	tnode		*pNodeRes;
	Column		thesaurus;
	Column		thesaurusRes;
	int			bAddValue;
	int			bCheck;	
	int			bLeftColumnRef;
	unsigned int i;
	short		op_tag;
	unsigned int	nTableId;
	unsigned int	nColumnId;
	unsigned int	nColumnSize;
	ColumnType		eColumnType;
	unsigned int	nLoopCnt;

	bool reverseOp = false;
	if ( pNode->tag == K_NOT ) {
		pNode = pNode->left;
		reverseOp = !reverseOp;
	}

	if ( is_column_reference( pNode->left ) != 0 ) {
		/* Left is column reference -> check for strings at right */
		pNodeColumnRef	= pNode->left;
		pNodeStr		= pNode->right;
		bLeftColumnRef	= 1;
		op_tag			= pNode->tag;
	} else if ( is_column_reference( pNode->right ) != 0 ) {
		/* Right is column reference -> check for strings at left */
		pNodeColumnRef	= pNode->right;
		pNodeStr		= pNode->left;
		bLeftColumnRef	= 0;
		/* Need to reverse the operations too ! */
		reverseOp = !reverseOp;
	}
	if( reverseOp )
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

	if ( get_thesaurus_info_for_column_reference( pNodeColumnRef, baseDesc, &nTableId, &nColumnId, &nColumnSize, &eColumnType, pErr ) == -1 )
		return pNode;

	/* Using column's thesaurus create a new one with those values which fulfills the condition ! */
	/* Keep the ColumnType ! */
	thesaurusRes.Type = eColumnType;
	if( thesaurusRes.Type == COL_TYPE_BIG_INT ) //debug13 - quick fix
		thesaurusRes.Type = COL_TYPE_INT;

	/* Loop on Thesaurus Parts (bigest nLoopCnt is 999, three digit !) */
	for ( nLoopCnt = 0; nLoopCnt < 1000; nLoopCnt++ ) {

		if ( get_thesaurus_for_column( thesaurus, nTableId, nColumnId, nColumnSize,
			nLoopCnt, eColumnType, pszPath, pErr ) != 0 ) {
			if ( nLoopCnt != 0 && *pErr == EXPR_TR_ERR_THESAURUS_FILE_NOT_FOUND ) {
				*pErr = 0;
				break;	/* No more Parts ! */
			}

			return pNode;	/* Keep old node */
		}

		/* for NOT add the values which are not in the interval, otherwise add those from interval */
		if ( op_tag == K_LT || op_tag == K_LEQ )
			bAddValue = 1;
		else if ( op_tag == K_GT || op_tag == K_GEQ )
			bAddValue = 0;

		bCheck = 1;
		for ( i = 0; i < thesaurus.Items.size(); i++ ) {
			if ( bCheck == 1 ) {
				/* Compare with the santinel : "Column_val cmp_op String_val" */ 
				if ( op_tag == K_LT || op_tag == K_GEQ ) {
					/* toggle state if >= */
					if ( thesaurus.Type == COL_TYPE_INT || thesaurus.Type == COL_TYPE_DATE1
						|| thesaurus.Type == COL_TYPE_DATE2 || thesaurus.Type == COL_TYPE_DATE3) {
						if ( pNodeStr->tag == K_INTEGER && thesaurus.Items[ i ]->numval >= pNodeStr->data.val_int 
							|| pNodeStr->tag == K_REAL && thesaurus.Items[ i ]->numval >= pNodeStr->data.val_number
							) {
							bCheck = 0;
							bAddValue = ( bAddValue == 0 ) ? 1 : 0;
						}
					} else if ( thesaurus.Type == COL_TYPE_DOUBLE ) {
						if ( pNodeStr->tag == K_REAL && thesaurus.Items[ i ]->numval >= pNodeStr->data.val_number
							|| pNodeStr->tag == K_INTEGER && thesaurus.Items[ i ]->numval >= pNodeStr->data.val_int ) {
							bCheck = 0;
							bAddValue = ( bAddValue == 0 ) ? 1 : 0;
						}
					} else { // if ( thesaurus.Type == COL_TYPE_VARCHAR )
						if ( strcmp( thesaurus.Items[ i ]->strval.c_str(), pNodeStr->data.val_str ) >= 0 ) {
							bCheck = 0;
							bAddValue = ( bAddValue == 0 ) ? 1 : 0;
						}
					}
				} else if ( op_tag == K_GT || op_tag == K_LEQ ) {
					/* toggle state if > */
					if ( thesaurus.Type == COL_TYPE_INT || thesaurus.Type == COL_TYPE_DATE1
						|| thesaurus.Type == COL_TYPE_DATE2 || thesaurus.Type == COL_TYPE_DATE3) {
						if ( thesaurus.Items[ i ]->numval > pNodeStr->data.val_int ) {
							bCheck = 0;
							bAddValue = ( bAddValue == 0 ) ? 1 : 0;
						}
					} else if ( thesaurus.Type == COL_TYPE_DOUBLE ) {
						if ( thesaurus.Items[ i ]->numval > pNodeStr->data.val_int ) {
							bCheck = 0;
							bAddValue = ( bAddValue == 0 ) ? 1 : 0;
						}
					} else { // if ( thesaurus.Type == COL_TYPE_VARCHAR )
						if ( strcmp( thesaurus.Items[ i ]->strval.c_str(), pNodeStr->data.val_str ) > 0 ) {
							bCheck = 0;
							bAddValue = ( bAddValue == 0 ) ? 1 : 0;
						}
					}
				}
			} else if ( bAddValue == 0 )
				break;		/* Nothing more to do ! */

			if ( bAddValue == 1 ) {
				std::vector<ColumnItem::Ptr>::const_iterator it = std::find_if(thesaurusRes.Items.begin(), thesaurusRes.Items.end(), 
					boost::bind(equal, boost::bind(&ColumnItem::Ptr::get, _1), thesaurus.Items[ i ].get(), thesaurus.Type));
				if (it == thesaurusRes.Items.end())
				{
					thesaurusRes.Items.push_back( thesaurus.Items[ i ] );
				}
			}
		}

	} /* & Loop on Thesaurus Parts */

	if ( thesaurusRes.Items.size() == 0 ) {
		/* No Match -> Expression evaluates to FALSE ! */
		pNodeRes = new_node( K_FALSE );
	} else if ( thesaurusRes.Items.size() == 1 ) {
		/* Replace it with operator = */
		if ( thesaurusRes.Type == COL_TYPE_INT || thesaurus.Type == COL_TYPE_DATE1
			|| thesaurus.Type == COL_TYPE_DATE2 || thesaurus.Type == COL_TYPE_DATE3) {
			pNodeRes = create_eq_subtree_integer( (llong) thesaurusRes.Items[ 0 ]->numval );
		} else if ( thesaurusRes.Type == COL_TYPE_DOUBLE ) {
			pNodeRes = create_eq_subtree_double( thesaurusRes.Items[ 0 ]->numval );
		} else { // if ( thesaurusRes.Type == COL_TYPE_VARCHAR )
			pNodeRes = create_eq_subtree_string( thesaurusRes.Items[ 0 ]->strval.c_str() );
		}
	} else {
		/* Replace it with operator IN */
		pNodeRes = create_in_subtree( 0, thesaurusRes );
	}

	if ( pNodeRes == NULL ) {
		/* Report Error ! */
		if ( pErr != NULL )
			*pErr = EXPR_TR_ERR_NOT_ENOUGH_MEMORY;
		return pNode;	/* Keep old node */
	}

	/* Success */
	if ( pNodeRes->tag != K_FALSE ) {
		/* Move the column reference subtree into the newly created subtree ! */
		pNodeRes->left = pNodeColumnRef;	/* which can be pNode->left or right ! */
		/* Remove reference from the original subtree which will be deleted ! */
		if ( bLeftColumnRef == 1 )
			pNode->left = NULL;
		else
			pNode->right = NULL;
	}

	/* Free pNode */
	delete_subtree( pNode );
	return pNodeRes;
}

//------------------------------------------------------------------------------
// PUBLIC API

//------------------------------------------------------------------------------
int get_thesaurus_for_column_reference( Column& thesaurus, tnode *pNode, int part, Base* baseDesc, char* pszPath, int *pErr ) {
	unsigned int	nTableId;
	unsigned int	nColumnId;
	unsigned int	nColumnSize;
	ColumnType		eColumnType;

	if ( get_thesaurus_info_for_column_reference( pNode, baseDesc, &nTableId, &nColumnId, &nColumnSize, &eColumnType, pErr ) == -1 )
		return -1;

	return get_thesaurus_for_column( thesaurus, nTableId, nColumnId, nColumnSize, part, eColumnType, pszPath, pErr );
}


//------------------------------------------------------------------------------
tnode* expression_transform( tnode *pNode, Base* baseDesc, char* pszPath, int *pErr ) {
	if ( pErr == NULL )
		return pNode;
	if ( pNode == NULL )
		return NULL;
	if ( baseDesc == NULL )
		return pNode;

	/* Need to detect :
	*	- (NOT)BETWEEN, <, >, <=, >=, (NOT)LIKE ( K_NOT, K_BETWEEN, K_LT, K_GT, K_LEQ, K_GEQ, K_LIKE )
	*	- operands suported : column_reference and strings (only strings for now!)
	* Transform to :
	*	- =, IN ( K_EQ, K_IN )
	*/

	/* Check for NOT BETWEEN or BETWEEN */
	if ( check_between_for_transform( pNode ) != 0 )
		return transform_between( pNode, baseDesc, pszPath, pErr );

	/* Check for NOT LIKE or LIKE */
	if ( check_like_for_transform( pNode ) != 0 ) 
		return transform_like( pNode, baseDesc, pszPath, pErr );

	/* Check for <, >, <=, >= */
	if ( check_cmp_op_for_transform( pNode ) != 0 )
		return transform_cmp_op( pNode, baseDesc, pszPath, pErr );

	/* Call recursively */
	pNode->next = expression_transform( pNode->next, baseDesc, pszPath, pErr );
	pNode->left = expression_transform( pNode->left, baseDesc, pszPath, pErr );
	/* Do not call on K_PERIOD's node right branch if the right tag is K_COLUMN ! */
	if ( pNode->right != NULL ) {
		if ( pNode->tag != K_PERIOD || pNode->right->tag != K_COLUMN ) {
			pNode->right = expression_transform( pNode->right, baseDesc, pszPath, pErr );
		}
	}

	return pNode;
}