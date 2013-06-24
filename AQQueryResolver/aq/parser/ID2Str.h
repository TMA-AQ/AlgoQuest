#ifndef __AQ_ID2STR_H__
#define __AQ_ID2STR_H__

#include "sql92_grm_tab.hpp"

namespace aq
{

//------------------------------------------------------------------------------
char* id_to_string( unsigned int nID );

//------------------------------------------------------------------------------
char* id_to_kstring( unsigned int nID );

//------------------------------------------------------------------------------
char* id_to_sql_string( unsigned int nID );

}

#endif /* __AQ_ID2STR_H__ */