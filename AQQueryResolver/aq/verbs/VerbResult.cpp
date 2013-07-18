#include "VerbResult.h"
#include <string.h>

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
const aq::data_holder_t Scalar::getValue() const
{
	aq::data_holder_t data;
	switch (Type)
	{
	case aq::COL_TYPE_INT:
	case aq::COL_TYPE_BIG_INT:
	case aq::COL_TYPE_DATE:
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

}
}
