#ifndef __AQ_VERB_RESULT_H__
#define __AQ_VERB_RESULT_H__

#include <aq/Object.h>
#include <aq/ColumnItem.h>
#include <aq/DBTypes.h>
#include <aq/RowProcess_Intf.h>

#include <vector>
#include <deque>

namespace aq {
namespace verb {

class VerbResult: public Object<VerbResult>
{
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
template <typename T>
class Scalar: public VerbResult
{
public:
  typedef boost::intrusive_ptr<Scalar<T> > Ptr;
  
	Scalar(aq::ColumnType type)
    : Type(type), aggFunc(aq::aggregate_function_t::NONE) 
  {
  }
	
  Scalar(aq::ColumnType type, unsigned int size)
    : Type(type), Size(size), aggFunc(aq::aggregate_function_t::NONE) 
  {
  }
	
  Scalar(aq::ColumnType type, unsigned int size, const aq::ColumnItem<T>& item)
    : Type(type), Item(item), Size(size), aggFunc(aq::aggregate_function_t::NONE) 
  {
  }
	
	virtual int getType() const 
  { 
    return VerbResult::SCALAR; 
  }

	const aq::data_holder_t getValue() const;

	std::string				       Name;
  aq::ColumnType           Type;
  aq::ColumnItem<T>        Item;
  unsigned int             Size;
  aq::aggregate_function_t aggFunc;

};

template <typename T>
const aq::data_holder_t Scalar<T>::getValue() const
{
  aq::data_holder_t data;
  data.val_int = Item.getValue();
  return data;
}

template <> inline
const aq::data_holder_t Scalar<double>::getValue() const
{
  aq::data_holder_t data;
  data.val_number = Item.getValue();
  return data;
}

template <> inline
const aq::data_holder_t Scalar<char*>::getValue() const
{
  aq::data_holder_t data;
  data.val_str = static_cast<char*>(::malloc((strlen(Item.getValue()) + 1) * sizeof(char)));
  strcpy(data.val_str, Item.getValue());
  return data;
}

//------------------------------------------------------------------------------
class SubTable: public VerbResult
{
public:
  typedef boost::intrusive_ptr<SubTable> Ptr;

	virtual int getType() const { return VerbResult::SUB_TABLE; }

	std::vector<int>	Rows;
};

//------------------------------------------------------------------------------
class Asterisk: public VerbResult
{
public:
  typedef boost::intrusive_ptr<Asterisk> Ptr;

	virtual int getType() const { return VerbResult::ASTERISK; }
};

//------------------------------------------------------------------------------
class TablePartition: public VerbResult
{
public:
  typedef boost::intrusive_ptr<TablePartition> Ptr;

	virtual int getType() const { return VerbResult::TABLE_PARTITION; }

	TablePartition()
		: FrameUnits(ROWS),
			FrameStart(0), FrameEnd(0),
	FrameStartType(AQ_UNBOUNDED),
	FrameEndType(AQ_UNBOUNDED),
	FrameUnitsInitialized(false)
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
	FrameUnitsType FrameUnits;
	long long FrameStart;
	long long FrameEnd;
	FrameBoundType FrameStartType;
	FrameBoundType FrameEndType;
	bool FrameUnitsInitialized;
};

//------------------------------------------------------------------------------
class RowValidation: public VerbResult
{
public:
  typedef boost::intrusive_ptr<RowValidation> Ptr;

	virtual int getType() const { return ROW_VALIDATION; };

	std::vector<bool> ValidRows;
};

//------------------------------------------------------------------------------
class VerbResultArray: public VerbResult
{
public:
  typedef boost::intrusive_ptr<VerbResultArray> Ptr;

	virtual int getType() const { return VerbResult::ARRAY; }

	std::deque<VerbResult::Ptr>	Results;
};

}
}

#endif
