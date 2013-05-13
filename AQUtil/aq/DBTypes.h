#ifndef __AQ_DB_TYPES_H__
#define __AQ_DB_TYPES_H__

namespace aq
{

enum aggregate_function_t // FIXME : not at the right place
{
  NONE,
  MIN,
  MAX,
  SUM,
  AVG,
  COUNT
};

//------------------------------------------------------------------------------
enum ColumnType
{
	COL_TYPE_VARCHAR	= 0,
	COL_TYPE_INT,
	COL_TYPE_BIG_INT,
	COL_TYPE_DOUBLE,
	COL_TYPE_DATE1,
	COL_TYPE_DATE2,
	COL_TYPE_DATE3,
	COL_TYPE_DATE4
};
	
union data_holder_t 
{
	char      *val_str;    // chaine
	long long	 val_int;    // entier
	double     val_number; // réel
} ;
//------------------------------------------------------------------------------
bool compatibleTypes( ColumnType type1, ColumnType type2 );

}

#endif