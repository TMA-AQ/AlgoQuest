#include "DBTypes.h"
#include <cassert>

namespace aq
{

//------------------------------------------------------------------------------
bool compatibleTypes( ColumnType type1, ColumnType type2 )
{
	switch( type1 )
	{
	case COL_TYPE_VARCHAR: return type2 == COL_TYPE_VARCHAR; break;
	case COL_TYPE_DOUBLE: 
	case COL_TYPE_INT: 
	case COL_TYPE_BIG_INT: 
		switch( type2 )
		{
		case COL_TYPE_VARCHAR: return type2 == COL_TYPE_VARCHAR; break;
		case COL_TYPE_DOUBLE: 
		case COL_TYPE_INT: 
		case COL_TYPE_BIG_INT: 
			return true;
		default:
			return false;
		}
		break;
	case COL_TYPE_DATE1:
	case COL_TYPE_DATE2:
	case COL_TYPE_DATE3:
	case COL_TYPE_DATE4:
		{
			switch( type2 )
			{
			case COL_TYPE_DATE1:
			case COL_TYPE_DATE2:
			case COL_TYPE_DATE3:
			case COL_TYPE_DATE4:
				return true;
			default:
				return false;
			}
		}
		break;
	}
	return false;
}

//------------------------------------------------------------------------------
const char * columnTypeToStr(ColumnType type)
{
  switch (type)
  {	
    case COL_TYPE_VARCHAR: return "CHA"; break;
    case COL_TYPE_INT: return "INT"; break;
    case COL_TYPE_DOUBLE: return "DOU"; break;
    case COL_TYPE_DATE1:
    case COL_TYPE_DATE2:
    case COL_TYPE_DATE3:
    case COL_TYPE_DATE4:
    case COL_TYPE_BIG_INT: return "LON"; break;
  }
  assert(false);
  return "";
}

}
