#include "VerbResult.h"

//------------------------------------------------------------------------------
const aq::data_holder_t Scalar::getValue() const
{
	aq::data_holder_t data;
	switch (Type)
	{
	case aq::COL_TYPE_INT:
	case aq::COL_TYPE_BIG_INT:
	case aq::COL_TYPE_DATE1:
	case aq::COL_TYPE_DATE2:
	case aq::COL_TYPE_DATE3:
	case aq::COL_TYPE_DATE4:
		data.val_int = static_cast<long long>(Item.numval);
		break;
	case aq::COL_TYPE_DOUBLE:
		data.val_number = Item.numval;
		break;
	case aq::COL_TYPE_VARCHAR:
		// if (data.val_str) free(data.val_str);
		data.val_str = static_cast<char*>(::malloc((Item.strval.size() + 1) * sizeof(char)));
		strcpy(data.val_str, Item.strval.c_str());
		break;
	default:
		break;
	}
	return data;
}
