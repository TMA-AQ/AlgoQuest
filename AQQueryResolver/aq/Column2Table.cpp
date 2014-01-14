#include "Column2Table.h"
#include "parser/sql92_grm_tab.hpp"
#include <aq/Base.h>
#include <aq/Logger.h>
#include <aq/Exceptions.h>
#include <boost/algorithm/string/case_conv.hpp>

namespace aq
{
  
///  
struct TColumn2Tables
{
	char			   *  m_pszColumnName;
	unsigned int	  m_nTableCount;
	char			   ** m_pparrTableNames;
};

///
struct TColumn2TablesArray
{
	unsigned int	    m_nColumnCount;
	TColumn2Tables ** m_pparrC2T;
};

//------------------------------------------------------------------------------
void delete_column2tables(aq::TColumn2Tables *pC2T) {
	if ( pC2T != nullptr ) {
		if ( pC2T->m_nTableCount != 0 ) {
			unsigned int iTable;
			for ( iTable = 0; iTable < pC2T->m_nTableCount; iTable++ ) {
				if ( pC2T->m_pparrTableNames[ iTable ] != nullptr ) {
					free( pC2T->m_pparrTableNames[ iTable ] );
					pC2T->m_pparrTableNames[ iTable ] = nullptr;
				}
			}
			free( pC2T->m_pparrTableNames );
		}
		if ( pC2T->m_pszColumnName != nullptr )
			free( pC2T->m_pszColumnName );
		free( pC2T );
	}
}

//------------------------------------------------------------------------------
void delete_column2tables_array( TColumn2TablesArray* parrC2T ) {
	if ( parrC2T != nullptr ) {
		if ( parrC2T->m_nColumnCount != 0 ) {
			unsigned int iColumn;
			for ( iColumn = 0; iColumn < parrC2T->m_nColumnCount; iColumn++ ) {
				if ( parrC2T->m_pparrC2T[ iColumn ] != nullptr ) {
					delete_column2tables( parrC2T->m_pparrC2T[ iColumn ] );
				}
			}
			free( parrC2T->m_pparrC2T );
		}
		free( parrC2T );
	}
}

//------------------------------------------------------------------------------
/* Return -1 on error or if not found, 0 on success */
int get_column_id_from_table(Table& pTD, char * pszColumnName, unsigned int * pnColumnId,
                             unsigned int * pnColumnSize, ColumnType * peColumnType ) {
	int idx = pTD.getColumnIdx( std::string(pszColumnName) );
	if( idx < 0 )
		return -1;

	if ( pnColumnId != nullptr )
		*pnColumnId = static_cast<unsigned int>(pTD.Columns[idx]->getID());
	if ( pnColumnSize != nullptr )
		*pnColumnSize = static_cast<unsigned int>(pTD.Columns[idx]->getSize());
	if ( peColumnType != nullptr )
		*peColumnType = pTD.Columns[idx]->getType();

	return 0;
}

//------------------------------------------------------------------------------
aq::TColumn2Tables * new_column2tables(const char * pszColumnName) 
{
	TColumn2Tables* pC2T;
	
	if ( pszColumnName == nullptr )
		return nullptr;

	pC2T = (TColumn2Tables*)malloc( sizeof( TColumn2Tables ) );
	if ( pC2T == nullptr ) {
    throw aq::generic_error(aq::generic_error::GENERIC, "Not enough memory");
		return nullptr;
	}
	memset( pC2T, 0, sizeof( TColumn2Tables ) );

	pC2T->m_pszColumnName = (char*)malloc( ( strlen( pszColumnName ) + 1 ) * sizeof( char ) );
	if ( pC2T->m_pszColumnName == nullptr ) {
    throw aq::generic_error(aq::generic_error::GENERIC, "Not enough memory");
		return nullptr;
	}

	strcpy( pC2T->m_pszColumnName, pszColumnName );
	boost::to_upper( pC2T->m_pszColumnName );
	return pC2T;
}

//------------------------------------------------------------------------------
aq::TColumn2Tables * add_table_name(aq::TColumn2Tables * pC2T, const char * pszTableName ) {
	char **ppTableNames;

	if ( pC2T == nullptr || pszTableName == nullptr ) {
#ifdef CREATE_LOG
		Log( "add_table_name() : One of the parameters is nullptr (pC2T:%u; pszTableName:<%u>)!\n", 
			 (unsigned int)pC2T, (unsigned int)pszTableName );
#endif
		return nullptr;
	}

	/* Allocate new pointer array */
	ppTableNames = (char**)malloc( ( pC2T->m_nTableCount + 1 ) * sizeof( char* ) );
	if ( ppTableNames == nullptr ) {
    throw aq::generic_error(aq::generic_error::GENERIC, "Not enough memory");
		return nullptr;
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
	if ( pC2T->m_pparrTableNames[ pC2T->m_nTableCount ] == nullptr ) {
    throw aq::generic_error(aq::generic_error::GENERIC, "Not enough memory");
		return nullptr;
	}

	/* Keep the column name */
	strcpy( pC2T->m_pparrTableNames[ pC2T->m_nTableCount ], pszTableName );
	boost::to_upper( pC2T->m_pparrTableNames[ pC2T->m_nTableCount ] );
	pC2T->m_nTableCount++;

	return pC2T;
}

//------------------------------------------------------------------------------
aq::TColumn2TablesArray * new_column2tables_array() {
	aq::TColumn2TablesArray * parrC2T;

	/* Allocate new pointer array */
	parrC2T = (aq::TColumn2TablesArray*)malloc( sizeof( aq::TColumn2TablesArray ) );
	if ( parrC2T == nullptr ) {
    throw aq::generic_error(aq::generic_error::GENERIC, "Not enough memory");
		return nullptr;
	}
	memset( parrC2T, 0, sizeof( aq::TColumn2TablesArray ) );
	return parrC2T;
}

//------------------------------------------------------------------------------
aq::TColumn2TablesArray * add_column2tables(aq::TColumn2TablesArray* parrC2T, aq::TColumn2Tables * pC2T) {
	aq::TColumn2Tables **ppC2TTmp;

	if ( pC2T == nullptr )
		return parrC2T;

	/* Allocate new pointer array */
	ppC2TTmp = (aq::TColumn2Tables**)malloc( ( parrC2T->m_nColumnCount + 1 ) * sizeof( aq::TColumn2Tables* ) );
	if ( ppC2TTmp == nullptr ) {
    throw aq::generic_error(aq::generic_error::GENERIC, "Not enough memory");
		return nullptr;
	}

	/* Copy old data if any & free the old buffer */
	if ( parrC2T->m_nColumnCount != 0 ) {
		memcpy( ppC2TTmp, parrC2T->m_pparrC2T, parrC2T->m_nColumnCount * sizeof( aq::TColumn2Tables* ) );
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
aq::TColumn2Tables * find_column_in_column2tables_array(aq::TColumn2TablesArray * parrC2T, const char * pszColumnName ) {
	unsigned int iColumn;
	char		 *pszTmpColumnName;

	if ( parrC2T == nullptr )
		return nullptr;
	if ( pszColumnName == nullptr )
		return nullptr;

	/* Create Temporary String to allow modification by strtoupr() */
	pszTmpColumnName = (char*)malloc( ( strlen( pszColumnName ) + 1 ) * sizeof( char ) );
	if ( pszTmpColumnName == nullptr ) {
    throw aq::generic_error(aq::generic_error::GENERIC, "Not enough memory");
		return nullptr;
	}
	strcpy( pszTmpColumnName, pszColumnName );
	boost::to_upper( pszTmpColumnName );

	for ( iColumn = 0; iColumn < parrC2T->m_nColumnCount; iColumn++ ) {
		if ( strcmp( parrC2T->m_pparrC2T[ iColumn ]->m_pszColumnName, pszTmpColumnName ) == 0 ) {
			/* Delete Temporary String */ 
			free( pszTmpColumnName );
			return parrC2T->m_pparrC2T[ iColumn ];
		}
	}

	/* Delete Temporary String */ 
	free( pszTmpColumnName );

	return nullptr;
}

//------------------------------------------------------------------------------
aq::TColumn2TablesArray * add_table_columns_to_column2tables_array(aq::TColumn2TablesArray * parrC2T, 
                                                                   aq::Base * baseDesc, 
                                                                   char * pszTableName ) {
	unsigned int iColumn;
	aq::TColumn2Tables *pC2T;
	
	Table& pTD = *baseDesc->getTable(pszTableName);

	for ( iColumn = 0; iColumn < pTD.Columns.size(); iColumn++ ) {
		pC2T = find_column_in_column2tables_array( parrC2T, pTD.Columns[ iColumn ]->getName().c_str() );
		if ( pC2T == nullptr ) {
			/* Add new Column2Table */
			pC2T = new_column2tables( pTD.Columns[ iColumn ]->getName().c_str() );
			if ( pC2T == nullptr )
				return nullptr;
			if ( add_column2tables( parrC2T, pC2T ) == nullptr ) {
				delete_column2tables( pC2T );
				return nullptr;
			}
		}
		/* Column already found or new created - add this table name to it ! */
		if ( add_table_name( pC2T, pTD.getName().c_str() ) == nullptr ) {
#ifdef CREATE_LOG
			Log( "add_table_columns_to_column2tables_array() : Function add_table_name( T:<%s> ) returned nullptr !\n", pTD.getName().c_str() );
#endif
			return nullptr;
		}
	}

	return parrC2T;
}

//------------------------------------------------------------------------------
// Return -1 on error, 0 on success
int add_tnode_tables(aq::tnode * pNode, aq::Base * baseDesc, aq::TColumn2TablesArray * parrC2T) 
{
	/* Check node type against K_COMMA, K_IDENT */
	if ( pNode->tag == K_COMMA ) 
  {
		if ( add_tnode_tables( pNode->left, baseDesc, parrC2T ) != 0 ) 
			return -1;
		if ( add_tnode_tables( pNode->right, baseDesc, parrC2T ) != 0 ) 
			return -1;
	} 
  else if ( pNode->tag == K_IDENT ) 
  {
		if ( add_table_columns_to_column2tables_array( parrC2T, baseDesc, pNode->getData().val_str ) == nullptr ) 
    {
      aq::Logger::getInstance().log(AQ_ERROR, "add_aq::tnode_tables() : Function add_table_columns_to_column2tables_array() returned -1 (error)!\n");
			return -1;
		}
	} 
  else if ( pNode->tag == K_INNER || pNode->tag == K_LEFT || pNode->tag == K_RIGHT || 
    pNode->tag == K_FULL || pNode->tag == K_OUTER || pNode->tag == K_JOIN ) 
  {
		/* K_INNER, K_LEFT, K_RIGHT, K_FULL, K_OUTER, K_JOIN */
    if ( add_tnode_tables( pNode->left, baseDesc, parrC2T ) != 0 ) 
			return -1;
	} 
  else if ( pNode->tag == K_AS ) 
  {
		/* correlation_name */
		if ( add_tnode_tables( pNode->left, baseDesc, parrC2T ) != 0 ) 
			return -1;
	} 
  else if ( pNode->tag == K_SELECT ) 
  {
		/* K_SELECT -> find K_FROM */
		aq::tnode *pNodeFound;
		pNodeFound = pNode->find_main(K_FROM );
		if ( pNodeFound == nullptr ) 
    {
      aq::Logger::getInstance().log(AQ_ERROR, "add_aq::tnode_tables() : Function find_main_node() returned nullptr !\n");
			return -1;
		}
		if ( add_tnode_tables( pNodeFound->left, baseDesc, parrC2T ) != 0 )
			return -1;
	} 
  else 
  {
		aq::Logger::getInstance().log(AQ_ERROR, "add_aq::tnode_tables() : Invalid TAG (%u) encountered !\n", pNode->tag);
		return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
TColumn2TablesArray * create_column_map_for_tables_used_in_select(aq::tnode * pNode, Base * baseDesc)
{
	aq::tnode * pNodeFound = nullptr;
	TColumn2TablesArray * parrC2T = nullptr;

	pNodeFound = pNode->find_main(K_FROM);
	if ( pNodeFound == nullptr ) 
  {
		aq::Logger::getInstance().log(AQ_ERROR, "create_column_map_for_tables_used_in_select() : Function find_main_node() returned nullptr !\n");
		return nullptr;
	}

	parrC2T = new_column2tables_array();
	if ((parrC2T != nullptr) && (add_tnode_tables( pNodeFound->left, baseDesc, parrC2T ) != 0)) 
  {
    aq::Logger::getInstance().log(AQ_ERROR, "add_tnode_tables failed\n");
		delete_column2tables_array(parrC2T);
    parrC2T = nullptr;
	}
	
	return parrC2T;
}

//------------------------------------------------------------------------------
int enforce_qualified_column_reference(aq::tnode * pNode, aq::Base & baseDesc) 
{
  int pErr = 0;

  TColumn2TablesArray * parrC2T = create_column_map_for_tables_used_in_select(pNode, &baseDesc);
  
	if ( parrC2T == nullptr )
		return -1;

	if ( pNode == nullptr )
  {
    delete_column2tables_array(parrC2T);
		return -1;
  }

	std::vector<aq::tnode*> nodes;
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
		if(	pNode->right && (pNode->tag != K_PERIOD || pNode->right->tag != K_COLUMN ) )
				nodes.push_back( pNode->right );
		
		if ( pNode->tag != K_COLUMN ) 
			continue;
		//reached a K_COLUMN without a K_PERIOD parent
		TColumn2Tables* pC2T;
		pC2T = find_column_in_column2tables_array( parrC2T, pNode->getData().val_str );
		if ( pC2T != nullptr ) 
    {
			if ( pC2T->m_nTableCount > 1 ) 
      {
        delete_column2tables_array(parrC2T);
				aq::Logger::getInstance().log(AQ_ERROR, "Column name ambiguity ! Multiple tables with same column name : <%s>", pNode->getData().val_str);
				return -1;
			} 
      else if ( pC2T->m_nTableCount == 0 ) 
      {
        delete_column2tables_array(parrC2T);
				aq::Logger::getInstance().log(AQ_ERROR, "No table with column name <%s> specified using FROM ... !", pNode->getData().val_str);
        return -2;
			} 
      else 
      {
				aq::tnode *pNodeColumn, *pNodeTable;
				pNodeColumn = new aq::tnode( *pNode );
				if ( pNodeColumn == nullptr ) 
        {
          delete_column2tables_array(parrC2T);
					return -3;
				}
				pNodeTable = new aq::tnode( K_IDENT );
				if ( pNodeTable == nullptr ) 
        {
          delete_column2tables_array(parrC2T);
          delete pNodeColumn ;
					return -4;
				}
				pNodeTable->set_string_data( pC2T->m_pparrTableNames[ 0 ] );
				pNode->tag = K_PERIOD;
				pNode->set_int_data(0);
				pNode->left	= pNodeTable;
				pNode->right = pNodeColumn;
			}
		}
	}

  delete_column2tables_array(parrC2T);
  return pErr;
}

}
