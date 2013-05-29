#include "ColumnItem.h"
#include <aq/Utilities.h>
#include <aq/DateConversion.h>
#include <aq/Exceptions.h>

namespace aq
{

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
bool ColumnItem::lessThan( const ColumnItem * first, const ColumnItem * second, ColumnType type )
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
bool ColumnItem::lessThan( const ColumnItem& first, const ColumnItem& second )
{
	return (first.numval < second.numval) ||  (first.strval < second.strval);
}

//------------------------------------------------------------------------------
bool ColumnItem::equal( const ColumnItem * first, const ColumnItem * second, ColumnType type )
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
bool ColumnItem::equal( const ColumnItem& first, const ColumnItem& second )
{	
	return (first.numval == second.numval) && (first.strval == second.strval);
}

//------------------------------------------------------------------------------
void apply_aggregate(aggregate_function_t aggFunc, ColumnType type, ColumnItem& i1, uint64_t i1_count, const ColumnItem& i2, uint64_t i2_count)
{
  switch (aggFunc)
  {
  case MIN:
    if (ColumnItem::lessThan(&i2, &i1, type))
    {
      i1 = i2;
    }
    break;
  case MAX:
    if (ColumnItem::lessThan(&i1, &i2, type))
    {
      i1 = i2;
    }
    break;
  case SUM:
    if (type == ColumnType::COL_TYPE_VARCHAR)
    {
      throw aq::generic_error(aq::generic_error::INVALID_QUERY, "cannot apply sum on char type");
    }
    i1.numval += (i2_count * i2.numval);
    break;
  case AVG:
    if (type == ColumnType::COL_TYPE_VARCHAR)
    {
      throw aq::generic_error(aq::generic_error::INVALID_QUERY, "cannot apply avg on char type");
    }
    i1.numval = ((i1_count * i1.numval) + (i2_count * i2.numval)) / (i1_count + i2_count);
    break;
  case COUNT:
    throw aq::generic_error(aq::generic_error::NOT_IMPLEMENED, "aggregate count function is not implemented");
    break;
  }
}

}