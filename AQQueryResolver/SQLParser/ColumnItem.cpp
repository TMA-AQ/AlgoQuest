#include "ColumnItem.h"
#include <aq/Utilities.h>
#include <aq/DateConversion.h>
#include <aq/Exceptions.h>

using namespace aq;

//------------------------------------------------------------------------------
ColumnItem::ColumnItem()
	: numval(0),
		strval("")
{
}

//------------------------------------------------------------------------------
ColumnItem::ColumnItem( const ColumnItem& source)
	: numval(source.numval),
		strval(source.strval)
{
}

//------------------------------------------------------------------------------
ColumnItem& ColumnItem::operator=( const ColumnItem& source)
{
	if (this != &source)
	{
		this->numval = source.numval;
		this->strval = source.strval;
	}
	return *this;
}

//------------------------------------------------------------------------------
ColumnItem::ColumnItem( char* str, ColumnType type )
{
	switch( type )
	{
	case COL_TYPE_INT: 
	case COL_TYPE_BIG_INT:
	case COL_TYPE_DOUBLE:
		StrToDouble(str, &this->numval);
		break;
	case COL_TYPE_DATE1:
	case COL_TYPE_DATE2:
	case COL_TYPE_DATE3:
		{
			DateType dateType;
			llong intval;
			dateToBigInt( str, &dateType, &intval );
			this->numval = (double) intval;
		}
		break;
	case COL_TYPE_VARCHAR: 
		this->strval = str; 
		break;
	default:
		throw generic_error(generic_error::NOT_IMPLEMENED, "");
	}
}

//------------------------------------------------------------------------------
ColumnItem::ColumnItem( const std::string& strval )
{
	this->strval = strval;
}

//------------------------------------------------------------------------------
ColumnItem::ColumnItem( double numval )
{
	this->numval = numval;
}

//------------------------------------------------------------------------------
void ColumnItem::toString( char* buffer, const ColumnType& type ) const
{
	int res = 0;
	switch( type )
	{
		case COL_TYPE_BIG_INT:
		case COL_TYPE_INT:
			res = sprintf( buffer, "%lld", (llong) this->numval );
			break;
		case COL_TYPE_DOUBLE:
			doubleToString( buffer, this->numval );
			break;
		case COL_TYPE_DATE1:
			bigIntToDate( (long long) this->numval, DDMMYYYY_HHMMSS, buffer );
			break;
		case COL_TYPE_DATE2:
			bigIntToDate( (long long) this->numval, DDMMYYYY, buffer );
			break;
		case COL_TYPE_DATE3:
			bigIntToDate( (long long) this->numval, DDMMYY, buffer );
			break;
		case COL_TYPE_VARCHAR:
			res = sprintf( buffer, this->strval.c_str() );
			break;
		default:
			assert( 0 );
	}
	if( res < 0 )
		throw generic_error(generic_error::GENERIC, "");
}

//------------------------------------------------------------------------------
bool lessThan( ColumnItem * first, ColumnItem * second, ColumnType type )
{
	if( !first || !second )
		return false;
	switch( type )
	{
	case COL_TYPE_INT: 
	case COL_TYPE_BIG_INT:
	case COL_TYPE_DOUBLE:
	case COL_TYPE_DATE1:
	case COL_TYPE_DATE2:
	case COL_TYPE_DATE3:
		return first->numval < second->numval;
		break;
	case COL_TYPE_VARCHAR: 
		return first->strval < second->strval;
		break;
	default:
		throw generic_error(generic_error::NOT_IMPLEMENED, "");
	}
}

//------------------------------------------------------------------------------
bool lessThan( const ColumnItem& first, const ColumnItem& second )
{
	return (first.numval < second.numval) ||  (first.strval < second.strval);
}

//------------------------------------------------------------------------------
bool equal( ColumnItem * first, ColumnItem * second, ColumnType type )
{
	if( !first || !second )
		return false;
	switch( type )
	{
	case COL_TYPE_INT: 
	case COL_TYPE_BIG_INT:
	case COL_TYPE_DOUBLE:
	case COL_TYPE_DATE1:
	case COL_TYPE_DATE2:
	case COL_TYPE_DATE3:
		return first->numval == second->numval;
		break;
	case COL_TYPE_VARCHAR: 
		return first->strval == second->strval;
		break;
	default:
		throw generic_error(generic_error::NOT_IMPLEMENED, "");
	}
}

//------------------------------------------------------------------------------
bool equal( const ColumnItem& first, const ColumnItem& second )
{	
	return (first.numval == second.numval) && (first.strval == second.strval);
}
