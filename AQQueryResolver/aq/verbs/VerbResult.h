#ifndef __AQ_VERB_RESULT_H__
#define __AQ_VERB_RESULT_H__

#include <aq/Object.h>
#include <aq/ColumnItem.h>
#include <aq/DBTypes.h>
#include <aq/RowProcess_Intf.h>

#include <vector>
#include <deque>

class VerbResult: public Object
{
	OBJECT_DECLARE( VerbResult );
public:
	enum ResultType
	{
		COLUMN,
		SCALAR,
		TABLE_PARTITION,
		SUB_TABLE,
		ARRAY,
		ASTERISK,
		ROW_VALIDATION
	};
	virtual int getType() const = 0;
};

//------------------------------------------------------------------------------
class Scalar: public VerbResult
{
	OBJECT_DECLARE( Scalar );
public:

	virtual int getType() const { return VerbResult::SCALAR; }
	const aq::data_holder_t getValue() const;

	std::string				Name;

	Scalar( aq::ColumnType type ): Type(type), aggFunc(aq::aggregate_function_t::NONE) {}
	Scalar( aq::ColumnType type, unsigned int size, const ColumnItem& item ): Item(item), Type(type), Size(size), aggFunc(aq::aggregate_function_t::NONE) {}
	ColumnItem	Item;
	aq::ColumnType	Type;
  unsigned int Size;
  aq::aggregate_function_t aggFunc;

};

//------------------------------------------------------------------------------
class SubTable: public VerbResult
{
	OBJECT_DECLARE( SubTable );
public:
	virtual int getType() const { return VerbResult::SUB_TABLE; }

	std::vector<int>	Rows;
};

//------------------------------------------------------------------------------
class Asterisk: public VerbResult
{
	OBJECT_DECLARE( Asterisk );
public:
	virtual int getType() const { return VerbResult::ASTERISK; }
};

//------------------------------------------------------------------------------
class TablePartition: public VerbResult
{
	OBJECT_DECLARE( TablePartition );
public:
	virtual int getType() const { return VerbResult::TABLE_PARTITION; }

	TablePartition()
		: FrameUnits(ROWS), 
			FrameStartType(AQ_UNBOUNDED),
			FrameEndType(AQ_UNBOUNDED), 
			// FrameEndType(RELATIVE), 
			FrameStart(0), FrameEnd(0)
	{
	}

	std::vector<size_t>	Rows;
	//Column::Ptr	LastColumn; //last column by which partitioning was done

	//window frame specification
	enum FrameBoundType
	{
		AQ_RELATIVE,
		AQ_UNBOUNDED
	};
	enum FrameUnitsType
	{
		ROWS,
		RANGE
	};
	FrameUnitsType		FrameUnits;
	long long	FrameStart, FrameEnd;
	FrameBoundType	FrameStartType, FrameEndType;
	bool FrameUnitsInitialized;
};

//------------------------------------------------------------------------------
class RowValidation: public VerbResult
{
	OBJECT_DECLARE( RowValidation );
public:
	virtual int getType() const { return ROW_VALIDATION; };

	std::vector<bool> ValidRows;
};

//------------------------------------------------------------------------------
class VerbResultArray: public VerbResult
{
	OBJECT_DECLARE( VerbResultArray );
public:
	virtual int getType() const { return VerbResult::ARRAY; }

	std::deque<VerbResult::Ptr>	Results;
};

#endif
