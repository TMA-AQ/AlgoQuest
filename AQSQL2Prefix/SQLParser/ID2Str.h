#ifndef __FIAN_ID2STR_H__
#define __FIAN_ID2STR_H__

#include "sql92_grm_tab.h"

//------------------------------------------------------------------------------
typedef struct tagID2String {
	unsigned int	m_nID;
	char			*pszStr;
} TID2String;

//------------------------------------------------------------------------------
char* id_to_string( unsigned int nID );

//------------------------------------------------------------------------------
char* id_to_sql_string( unsigned int nID );

#endif /* __FIAN_ID2STR_H__ */