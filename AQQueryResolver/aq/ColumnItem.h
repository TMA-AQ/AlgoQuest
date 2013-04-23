#ifndef __AQ_COLUMN_ITEM_H__
#define __AQ_COLUMN_ITEM_H__

#include <aq/Object.h>
#include <aq/DBTypes.h>
#include <string>

//------------------------------------------------------------------------------
class ColumnItem: public Object
{
	OBJECT_DECLARE( ColumnItem );
public:
	// data_holder_t data;
	double numval;
	std::string strval;

	ColumnItem();
	ColumnItem( const ColumnItem& source);
	ColumnItem( char* strval, aq::ColumnType type );
	ColumnItem( const std::string& strval );
	ColumnItem( double numval );
	
	ColumnItem& operator=( const ColumnItem& source);
	void toString( char* buffer, const aq::ColumnType& type ) const;

//private:	
//	boost::variant<std::string, double> val;

};

bool lessThan( ColumnItem * first, ColumnItem * second, aq::ColumnType type );
bool lessThan( const ColumnItem& first, const ColumnItem& second );
bool equal( ColumnItem * first, ColumnItem * second, aq::ColumnType type );
bool equal( const ColumnItem& first, const ColumnItem& second );

struct column_cmp_t
{
	bool operator()(const ColumnItem& first, const ColumnItem& second)
	{
		return lessThan(first, second);
	}
	bool operator()(const ColumnItem::Ptr first, const ColumnItem::Ptr second)
	{
		return lessThan(*first.get(), *second.get());
	}
};


#endif