#ifndef __AQ_COLUMN_ITEM_H__
#define __AQ_COLUMN_ITEM_H__

#include <aq/Object.h>
#include <aq/DBTypes.h>
#include <string>
#include <cstdint>

namespace aq
{

//------------------------------------------------------------------------------
class ColumnItem: public Object
{
	OBJECT_DECLARE( ColumnItem );
public:
	// data_holder_t data;
	double numval;
	std::string strval;
  // void * value;

	ColumnItem();
	ColumnItem( const ColumnItem& source);
	ColumnItem( char* strval, aq::ColumnType type );
	ColumnItem( const std::string& strval );
	ColumnItem( double numval );
	
	ColumnItem& operator=( const ColumnItem& source);
	void toString( char* buffer, const aq::ColumnType& type ) const;
  std::string toString(const aq::ColumnType& type) const;

//private:	
//	boost::variant<std::string, double> val;

  // static bool lessThan( ColumnItem * first, ColumnItem * second, aq::ColumnType type );
  static bool lessThan( const ColumnItem * first, const ColumnItem * second, aq::ColumnType type );
  static bool lessThan( const ColumnItem& first, const ColumnItem& second );
  // static bool equal( ColumnItem * first, ColumnItem * second, aq::ColumnType type );
  static bool equal( const ColumnItem * first, const ColumnItem * second, aq::ColumnType type );
  static bool equal( const ColumnItem& first, const ColumnItem& second );

};

struct column_cmp_t
{
	bool operator()(const ColumnItem& first, const ColumnItem& second)
	{
		return ColumnItem::lessThan(first, second);
	}
	bool operator()(const ColumnItem::Ptr first, const ColumnItem::Ptr second)
	{
		return ColumnItem::lessThan(*first.get(), *second.get());
	}
};
    
 void apply_aggregate(aggregate_function_t aggFunc, ColumnType type, ColumnItem& i1, uint64_t i1_count, const ColumnItem& i2, uint64_t i2_count);

}

#endif