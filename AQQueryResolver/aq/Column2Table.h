#ifndef __AQ_COLUMN2TABLE_H__
#define __AQ_COLUMN2TABLE_H__

#include "parser/SQLParser.h"
#include "Table.h"

#define EXIT_ON_MEM_ERROR	1

//------------------------------------------------------------------------------
int loadFromBaseDesc( char* pszDataBaseDef, Base& baseDesc );

/* Return -1 on error, 0 on success */
int get_table_and_column_id_from_table_array( Base* baseDesc, char *pszTableName,
												  char *pszColumnName, unsigned int *pnTableId, 
												  unsigned int *pnColumnId,
												  unsigned int *pnColumnSize,
												  aq::ColumnType *peColumnType );

//------------------------------------------------------------------------------
typedef struct tagColumn2Tables {
	char			*m_pszColumnName;
	unsigned int	m_nTableCount;
	char			**m_pparrTableNames;
} TColumn2Tables;

//------------------------------------------------------------------------------
typedef struct tagColumn2TablesArray {
	unsigned int	m_nColumnCount;
	TColumn2Tables	**m_pparrC2T;
} TColumn2TablesArray;


//------------------------------------------------------------------------------
TColumn2TablesArray* create_column_map_for_tables_used_in_select( aq::tnode *pNode, Base* baseDesc );
void delete_column2tables_array( TColumn2TablesArray* parrC2T );

//------------------------------------------------------------------------------
void enforce_qualified_column_reference( aq::tnode *pNode, TColumn2TablesArray* parrC2T, int *pErr );

#endif /* __AQ_COLUMN2TABLE_H__ */