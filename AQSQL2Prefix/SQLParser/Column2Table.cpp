#include "Column2Table.h"
#include "sql92_grm_tab.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>		// One of this required by "toupper()"
#include <ctype.h>		// One of this required by "toupper()"

using namespace std;

//------------------------------------------------------------------------------
extern int yyerror ( const char *pszMsg );

//------------------------------------------------------------------------------
/* Return -1 on error or if not found, 0 on success */
int get_column_id_from_table( Table& pTD, char* pszColumnName, unsigned int *pnColumnId,
								  unsigned int *pnColumnSize, ColumnType *peColumnType ) {
	int idx = pTD.getColumnIdx( string(pszColumnName) );
	if( idx < 0 )
		return -1;

	if ( pnColumnId != NULL )
		*pnColumnId = pTD.Columns[idx]->ID;
	if ( pnColumnSize != NULL )
		*pnColumnSize = pTD.Columns[idx]->Size;
	if ( peColumnType != NULL )
		*peColumnType = pTD.Columns[idx]->Type;

	return 0;
}

//------------------------------------------------------------------------------
/* Return -1 on error, 0 on success */
int get_table_and_column_id_from_table_array( Base* baseDesc, char *pszTableName,
												  char *pszColumnName, unsigned int *pnTableId, 
												  unsigned int *pnColumnId, 
												  unsigned int *pnColumnSize,
												  ColumnType *peColumnType ) {
	
	int idx = baseDesc->getTableIdx( string(pszTableName) );
	if ( idx < 0 ) {
#ifdef CREATE_LOG
		Log( "get_table_and_column_id_from_table_array() : Function find_table_in_table_array( T:<%s> ) returned NULL !\n", pszTableName );
#endif
		return -1;
	}

	if ( pnTableId != NULL )
		*pnTableId = baseDesc->Tables[idx].ID;

	if ( get_column_id_from_table( baseDesc->Tables[idx], pszColumnName, pnColumnId, pnColumnSize, peColumnType ) != 0 ) {
#ifdef CREATE_LOG
		Log( "get_table_and_column_id_from_table_array() : Function get_column_id_from_table( T:<%s>, C:<%s> ) returned NULL !\n", pszTableName, pszColumnName );
#endif
		return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
TColumn2Tables* new_column2tables( const char *pszColumnName ) {
	TColumn2Tables* pC2T;
	
	if ( pszColumnName == NULL )
		return NULL;

	pC2T = (TColumn2Tables*)malloc( sizeof( TColumn2Tables ) );
	if ( pC2T == NULL ) {
		report_error( "Not enough memory", EXIT_ON_MEM_ERROR );
		return NULL;
	}
	memset( pC2T, 0, sizeof( TColumn2Tables ) );

	pC2T->m_pszColumnName = (char*)malloc( ( strlen( pszColumnName ) + 1 ) * sizeof( char ) );
	if ( pC2T->m_pszColumnName == NULL ) {
		report_error( "Not enough memory", EXIT_ON_MEM_ERROR );
		return NULL;
	}

	strcpy( pC2T->m_pszColumnName, pszColumnName );
	strtoupr( pC2T->m_pszColumnName );
	return pC2T;
}

//------------------------------------------------------------------------------
void delete_column2tables( TColumn2Tables *pC2T ) {
	if ( pC2T != NULL ) {
		if ( pC2T->m_nTableCount != 0 ) {
			unsigned int iTable;
			for ( iTable = 0; iTable < pC2T->m_nTableCount; iTable++ ) {
				if ( pC2T->m_pparrTableNames[ iTable ] != NULL ) {
					free( pC2T->m_pparrTableNames[ iTable ] );
					pC2T->m_pparrTableNames[ iTable ] = NULL;
				}
			}
			free( pC2T->m_pparrTableNames );
		}
		if ( pC2T->m_pszColumnName != NULL )
			free( pC2T->m_pszColumnName );
		free( pC2T );
	}
}

//------------------------------------------------------------------------------
TColumn2Tables* add_table_name( TColumn2Tables *pC2T, const char *pszTableName ) {
	char **ppTableNames;

	if ( pC2T == NULL || pszTableName == NULL ) {
#ifdef CREATE_LOG
		Log( "add_table_name() : One of the parameters is NULL (pC2T:%u; pszTableName:<%u>)!\n", 
			 (unsigned int)pC2T, (unsigned int)pszTableName );
#endif
		return NULL;
	}

	/* Allocate new pointer array */
	ppTableNames = (char**)malloc( ( pC2T->m_nTableCount + 1 ) * sizeof( char* ) );
	if ( ppTableNames == NULL ) {
		report_error( "Not enough memory", EXIT_ON_MEM_ERROR );
		return NULL;
	}

	/* Copy old data if any & free the old buffer */
	if ( pC2T->m_nTableCount != 0 ) {
		memcpy( ppTableNames, pC2T->m_pparrTableNames, pC2T->m_nTableCount * sizeof( char* ) );
		free( pC2T->m_pparrTableNames );
	}

	/* Set the new buffer */
	pC2T->m_pparrTableNames = ppTableNames;

	/* Allocate buffer for column name */
	pC2T->m_pparrTableNames[ pC2T->m_nTableCount ] = (char*)malloc( strlen( pszTableName ) + 1 );
	if ( pC2T->m_pparrTableNames[ pC2T->m_nTableCount ] == NULL ) {
		report_error( "Not enough memory", EXIT_ON_MEM_ERROR );
		return NULL;
	}

	/* Keep the column name */
	strcpy( pC2T->m_pparrTableNames[ pC2T->m_nTableCount ], pszTableName );
	strtoupr( pC2T->m_pparrTableNames[ pC2T->m_nTableCount ] );
	pC2T->m_nTableCount++;

	return pC2T;
}

//------------------------------------------------------------------------------
TColumn2TablesArray* new_column2tables_array( void ) {
	TColumn2TablesArray *parrC2T;

	/* Allocate new pointer array */
	parrC2T = (TColumn2TablesArray*)malloc( sizeof( TColumn2TablesArray ) );
	if ( parrC2T == NULL ) {
		report_error( "Not enough memory", EXIT_ON_MEM_ERROR );
		return NULL;
	}
	memset( parrC2T, 0, sizeof( TColumn2TablesArray ) );
	return parrC2T;
}

//------------------------------------------------------------------------------
void delete_column2tables_array( TColumn2TablesArray* parrC2T ) {
	if ( parrC2T != NULL ) {
		if ( parrC2T->m_nColumnCount != 0 ) {
			unsigned int iColumn;
			for ( iColumn = 0; iColumn < parrC2T->m_nColumnCount; iColumn++ ) {
				if ( parrC2T->m_pparrC2T[ iColumn ] != NULL ) {
					delete_column2tables( parrC2T->m_pparrC2T[ iColumn ] );
				}
			}
			free( parrC2T->m_pparrC2T );
		}
		free( parrC2T );
	}
}

//------------------------------------------------------------------------------
TColumn2TablesArray* add_column2tables( TColumn2TablesArray* parrC2T, TColumn2Tables* pC2T ) {
	TColumn2Tables **ppC2TTmp;

	if ( pC2T == NULL )
		return parrC2T;

	/* Allocate new pointer array */
	ppC2TTmp = (TColumn2Tables**)malloc( ( parrC2T->m_nColumnCount + 1 ) * sizeof( TColumn2Tables* ) );
	if ( ppC2TTmp == NULL ) {
		report_error( "Not enough memory", EXIT_ON_MEM_ERROR );
		return NULL;
	}

	/* Copy old data if any & free the old buffer */
	if ( parrC2T->m_nColumnCount != 0 ) {
		memcpy( ppC2TTmp, parrC2T->m_pparrC2T, parrC2T->m_nColumnCount * sizeof( TColumn2Tables* ) );
		free( parrC2T->m_pparrC2T );
	}

	/* Set the new buffer */
	parrC2T->m_pparrC2T = ppC2TTmp;

	/* Set the new data */
	parrC2T->m_pparrC2T[ parrC2T->m_nColumnCount ] = pC2T;
	parrC2T->m_nColumnCount++;

	return parrC2T;
}

//------------------------------------------------------------------------------
TColumn2Tables* find_column_in_column2tables_array( TColumn2TablesArray* parrC2T, const char* pszColumnName ) {
	unsigned int iColumn;
	char		 *pszTmpColumnName;

	if ( parrC2T == NULL )
		return NULL;
	if ( pszColumnName == NULL )
		return NULL;

	/* Create Temporary String to allow modification by strtoupr() */
	pszTmpColumnName = (char*)malloc( ( strlen( pszColumnName ) + 1 ) * sizeof( char ) );
	if ( pszTmpColumnName == NULL ) {
		report_error( "Not enough memory", EXIT_ON_MEM_ERROR );
		return NULL;
	}
	strcpy( pszTmpColumnName, pszColumnName );
	strtoupr( pszTmpColumnName );

	for ( iColumn = 0; iColumn < parrC2T->m_nColumnCount; iColumn++ ) {
		if ( strcmp( parrC2T->m_pparrC2T[ iColumn ]->m_pszColumnName, pszTmpColumnName ) == 0 ) {
			/* Delete Temporary String */ 
			free( pszTmpColumnName );
			return parrC2T->m_pparrC2T[ iColumn ];
		}
	}

	/* Delete Temporary String */ 
	free( pszTmpColumnName );

	return NULL;
}

//------------------------------------------------------------------------------
TColumn2TablesArray* add_table_columns_to_column2tables_array(	TColumn2TablesArray* parrC2T, 
																Base* baseDesc, 
																char *pszTableName ) {
	unsigned int iColumn;
	TColumn2Tables *pC2T;
	
	int tableIdx = baseDesc->getTableIdx( string(pszTableName) );
	if ( tableIdx < 0 ) {
#ifdef CREATE_LOG
		Log( "add_table_columns_to_column2tables_array() : Function find_table_in_table_array( T:<%s> ) returned NULL !\n", pszTableName );
#endif
		return NULL;
	}
	Table& pTD = baseDesc->Tables[tableIdx];

	for ( iColumn = 0; iColumn < pTD.Columns.size(); iColumn++ ) {
		pC2T = find_column_in_column2tables_array( parrC2T, pTD.Columns[ iColumn ]->getName().c_str() );
		if ( pC2T == NULL ) {
			/* Add new Column2Table */
			pC2T = new_column2tables( pTD.Columns[ iColumn ]->getName().c_str() );
			if ( pC2T == NULL )
				return NULL;
			if ( add_column2tables( parrC2T, pC2T ) == NULL ) {
				delete_column2tables( pC2T );
				return NULL;
			}
		}
		/* Column already found or new created - add this table name to it ! */
		if ( add_table_name( pC2T, pTD.getName().c_str() ) == NULL ) {
#ifdef CREATE_LOG
			Log( "add_table_columns_to_column2tables_array() : Function add_table_name( T:<%s> ) returned NULL !\n", pTD.getName().c_str() );
#endif
			return NULL;
		}
	}

	return parrC2T;
}

//------------------------------------------------------------------------------
/* Return -1 on error, 0 on success */
int add_tnode_tables( tnode *pNode, Base* baseDesc, TColumn2TablesArray* parrC2T ) {
	/* Check node type against K_COMMA, K_IDENT */
	if ( pNode->tag == K_COMMA ) {
		if ( add_tnode_tables( pNode->left, baseDesc, parrC2T ) != 0 ) 
			return -1;
		if ( add_tnode_tables( pNode->right, baseDesc, parrC2T ) != 0 ) 
			return -1;
	} else if ( pNode->tag == K_IDENT ) {
		if ( add_table_columns_to_column2tables_array( parrC2T, baseDesc, pNode->data.val_str ) == NULL ) {
#ifdef CREATE_LOG
		Log( "add_tnode_tables() : Function add_table_columns_to_column2tables_array() returned -1 (error)!\n" );
#endif
			return -1;
		}
	} else if ( pNode->tag == K_INNER || pNode->tag == K_LEFT || pNode->tag == K_RIGHT || 
				pNode->tag == K_FULL || pNode->tag == K_OUTER || pNode->tag == K_JOIN ) {
		/* K_INNER, K_LEFT, K_RIGHT, K_FULL, K_OUTER, K_JOIN */
		if ( add_tnode_tables( pNode->left, baseDesc, parrC2T ) != 0 ) 
			return -1;
	} else if ( pNode->tag == K_AS ) {
		/* correlation_name */
		if ( add_tnode_tables( pNode->left, baseDesc, parrC2T ) != 0 ) 
			return -1;
	} else if ( pNode->tag == K_SELECT ) {
		/* K_SELECT -> find K_FROM */
		tnode *pNodeFound;
		pNodeFound = find_main_node( pNode, K_FROM );
		if ( pNodeFound == NULL ) {
#ifdef CREATE_LOG
			Log( "add_tnode_tables() : Function find_main_node() returned NULL !\n" );
#endif
			return -1;
		}
		if ( add_tnode_tables( pNodeFound->left, baseDesc, parrC2T ) != 0 )
			return -1;
	} else {
#ifdef CREATE_LOG
		Log( "add_tnode_tables() : Invalid TAG (%u) encountered !\n", pNode->tag );
#endif
		return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
TColumn2TablesArray* create_column_map_for_tables_used_in_select( tnode *pNode, Base* baseDesc ) {
	tnode *pNodeFound;
	TColumn2TablesArray* parrC2T;

	/* Find K_FROM node */
	pNodeFound = find_main_node( pNode, K_FROM );
	if ( pNodeFound == NULL ) {
#ifdef CREATE_LOG
		Log( "create_column_map_for_tables_used_in_select() : Function find_main_node() returned NULL !\n" );
#endif
		return NULL;
	}

	parrC2T = new_column2tables_array();
	if ( parrC2T == NULL )
		return NULL;

	if ( add_tnode_tables( pNodeFound->left, baseDesc, parrC2T ) != 0 ) {
		delete_column2tables_array( parrC2T );
		return NULL;
	}
	
	return parrC2T;
}

//------------------------------------------------------------------------------
void enforce_qualified_column_reference( tnode *pNode, TColumn2TablesArray* parrC2T, int *pErr ) {
	if ( pErr == NULL )
		return;

	if ( pNode == NULL )
		return;

	if ( parrC2T == NULL )
		return;

	vector<tnode*> nodes;
	nodes.push_back( pNode );
	size_t idx = 0;
	while( idx < nodes.size() )
	{
		pNode = nodes[idx];
		++idx;
		if( pNode->next )
			nodes.push_back( pNode->next );
		if( pNode->left )
			nodes.push_back( pNode->left );
		//do not call on K_PERIOD's node right branch if the right tag is K_COLUMN !
		if(	pNode->right &&
			(pNode->tag != K_PERIOD || pNode->right->tag != K_COLUMN ) )
				nodes.push_back( pNode->right );
		
		if ( pNode->tag != K_COLUMN ) 
			continue;
		//reached a K_COLUMN without a K_PERIOD parent
		TColumn2Tables* pC2T;
		pC2T = find_column_in_column2tables_array( parrC2T, pNode->data.val_str );
		if ( pC2T != NULL ) {
			if ( pC2T->m_nTableCount > 1 ) {
				char szBuf[ 1000 ];
				/* Error column name ambiguity ! */
				sprintf( szBuf, "Column name ambiguity ! Multiple tables with same column name : <%s>", pNode->data.val_str );
				yyerror( szBuf );
				*pErr = -1;
			} else if ( pC2T->m_nTableCount == 0 ) {
				char szBuf[ 1000 ];
				/* Error no table with column name */
				sprintf( szBuf, "No table with column name <%s> specified using FROM ... !", pNode->data.val_str );
				yyerror( szBuf );
				*pErr = -2;
			} else {
				tnode *pNodeColumn, *pNodeTable;
				pNodeColumn = new_node( pNode );
				if ( pNodeColumn == NULL ) {
					*pErr = -3;
					return;
				}
				pNodeTable = new_node( K_IDENT );
				if ( pNodeTable == NULL ) {
					delete_node( pNodeColumn );
					*pErr = -4;
					return;
				}
				if ( set_string_data( pNodeTable, pC2T->m_pparrTableNames[ 0 ] ) == NULL ) {
					delete_node( pNodeColumn );
					delete_node( pNodeTable );
					*pErr = -5;
					return;
				}
				pNode->tag = K_PERIOD;
				free( pNode->data.val_str );
				pNode->eNodeDataType = NODE_DATA_INT;
				pNode->left	= pNodeTable;
				pNode->right = pNodeColumn;
			}
		}
	}
}


