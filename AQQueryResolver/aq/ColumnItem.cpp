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
	case COL_TYPE_DATE:
		{
			DateConversion dateConverter;
			llong intval = dateConverter.dateToBigInt(str);
			this->numval = (double) intval;
		}
		break;
	case COL_TYPE_VARCHAR: 
		this->strval = str; 
		break;
	default:
		throw generic_error(generic_error::NOT_IMPLEMENTED, "type not supported");
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
		case COL_TYPE_DATE:
      {
        DateConversion dateConverter;
        std::string date_str = dateConverter.bigIntToDate((long long) this->numval);
        strcpy(buffer, date_str.c_str());
      }
			break;
		case COL_TYPE_VARCHAR:
                  res = sprintf( buffer, "%s", this->strval.c_str() );
			break;
		default:
			assert( 0 );
	}
	if( res < 0 )
		throw generic_error(generic_error::GENERIC, "");
}

std::string ColumnItem::toString(const aq::ColumnType& type) const
{
  std::stringstream ss;
  switch( type )
  {
  case COL_TYPE_BIG_INT:
  case COL_TYPE_INT:
    ss << static_cast<int>(this->numval);
    return ss.str();
  case COL_TYPE_DOUBLE:
    ss << this->numval;
    return ss.str();
  case COL_TYPE_DATE:
    {
      DateConversion dateConverter;
      return dateConverter.bigIntToDate((long long) this->numval);
    }
  case COL_TYPE_VARCHAR:
    return this->strval;
  }
  assert(false);
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
	case COL_TYPE_DATE:
		return first->numval < second->numval;
		break;
	case COL_TYPE_VARCHAR: 
		return strcmp(first->strval.c_str(), second->strval.c_str()) < 0;
		break;
	default:
		throw generic_error(generic_error::NOT_IMPLEMENTED, "type not supported");
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
	case COL_TYPE_DATE:
		return first->numval == second->numval;
		break;
	case COL_TYPE_VARCHAR: 
		return first->strval == second->strval;
		break;
	default:
		throw generic_error(generic_error::NOT_IMPLEMENTED, "type not supported");
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
    throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "aggregate count function is not implemented");
    // i1.numval += i2_count;
    break;
  }
}

}
