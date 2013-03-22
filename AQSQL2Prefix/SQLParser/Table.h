#ifndef __FIAN_TABLE_H__
#define __FIAN_TABLE_H__

#include "Object.h"
#include "Settings.h"
#include "AQMatrix.h"

#include <aq/DBTypes.h>
#include <aq/Utilities.h>

#include <vector>
#include <string>
#include <deque>

#include <boost/variant.hpp>

// Forward declaration (need because of a conflict #define in header)
namespace aq 
{
	class FileMapper;
}

//------------------------------------------------------------------------------
class Base;

//------------------------------------------------------------------------------
#define EXPR_TR_ERR_NOT_COLUMN_REFERENCE			-2
#define EXPR_TR_ERR_TBL_OR_COL_ID_NOT_FOUND			-3
#define EXPR_TR_ERR_LOADING_THESAURUS				-4
#define EXPR_TR_ERR_NOT_ENOUGH_MEMORY				-5
#define EXPR_TR_ERR_NO_MATCH						-6
#define EXPR_TR_ERR_CREATING_THESAURUS				-7
#define EXPR_TR_ERR_THESAURUS_FILE_NOT_FOUND		-8
#define EXPR_TR_ERR_READING_THESAURUS_FILE			-9
#define EXPR_TR_ERR_INVALID_THESAURUS_FILE			-10
#define EXPR_TR_ERR_PREPARING_PATTERN_MATCHING		-11
#define EXPR_TR_ERR_PATTERN_MATCHING				-12

//------------------------------------------------------------------------------
class ColumnItem: public Object
{
	OBJECT_DECLARE( ColumnItem );
public:
	// data_holder_t data;
	double numval;
	std::string strval;

	ColumnItem();
	ColumnItem( char* strval, aq::ColumnType type );
	ColumnItem( const std::string& strval );
	ColumnItem( double numval );

	void toString( char* buffer, const aq::ColumnType& type ) const;

private:	
	boost::variant<std::string, double> val;

};

bool lessThan( ColumnItem* first, ColumnItem* second, aq::ColumnType type );
bool equal( ColumnItem* first, ColumnItem* second, aq::ColumnType type );

//------------------------------------------------------------------------------
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
	// virtual int getType(){ assert(0); return -1; }; //abstract class semantics
	virtual int getType() const = 0; // real abstract class
};

//------------------------------------------------------------------------------
class Column: public VerbResult 
{
	OBJECT_DECLARE( Column );
public:
	virtual int getType() const { return VerbResult::COLUMN; }

	Column();
	Column( aq::ColumnType type );
	Column(	const std::string& name, unsigned int ID,
			unsigned int size, aq::ColumnType type);
	Column( const Column& source );
	Column& operator=(const Column& source);

	void setName( const std::string& name );
	void setDisplayName( const std::string& name );
	std::string& getName();
	std::string& getDisplayName();
	std::string& getOriginalName();

	void setTableName( const std::string& name );
	std::string& getTableName();

	int loadFromThesaurus( const char *pszFilePath, int nFileType, 
		unsigned int nColumnSize, aq::ColumnType eColumnType, int *pErr );
	void increase( size_t newSize );
	void setCount( Column::Ptr count );
	Column::Ptr getCount();

	void addItem(size_t index, const TProjectSettings& pSettings, const Base& BaseDesc);

	void loadFromFile( const std::string& file );
	//endIdx == -1 means 'number of items in the column'
	void saveToFile(	const std::string& file, 
						int startIdx = 0, int endIdx = -1, 
						bool append = false );
  
	void dump( std::ostream& os );

	std::vector<ColumnItem::Ptr>	Items;
	unsigned int	TableID;
	unsigned int	ID;
	unsigned int	Size;	//maximum size of the text, not number of items
	aq::ColumnType		Type;
	
private:

	void setBinItemSize();

	std::string	Name;
	std::string	OriginalName;
	std::string	DisplayName;
	Column::Ptr	Count;	//reference to count column
	std::string	TableName;

	boost::shared_ptr<aq::FileMapper> prmMapper;
	boost::shared_ptr<aq::FileMapper> thesaurusMapper;

	size_t prmFileItemSize;
	size_t currentNumPack;
	size_t packOffset;
	size_t nBinItemSize;
	char * pTmpBuf;

public:
	bool			Invisible;
	bool			GroupBy;
	bool			OrderBy;

};

//------------------------------------------------------------------------------
class Scalar: public VerbResult
{
	OBJECT_DECLARE( Scalar );
public:
	virtual int getType() const { return VerbResult::SCALAR; }

	std::string				Name;

	Scalar( aq::ColumnType type ): Type(type){}
	Scalar( aq::ColumnType type, const ColumnItem& item ): Type(type), Item(item){}
	ColumnItem	Item;
	aq::ColumnType	Type;

	const aq::data_holder_t getValue() const
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
			data.val_int = Item.numval;
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
			FrameStartType(UNBOUNDED),
			FrameEndType(UNBOUNDED), 
			// FrameEndType(RELATIVE), 
			FrameStart(0), FrameEnd(0)
	{
	}

	std::vector<int>	Rows;
	//Column::Ptr	LastColumn; //last column by which partitioning was done

	//window frame specification
	enum FrameBoundType
	{
		RELATIVE,
		UNBOUNDED
	};
	enum FrameUnitsType
	{
		ROWS,
		RANGE
	};
	FrameUnitsType		FrameUnits;
	llong	FrameStart, FrameEnd;
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
namespace aq
{
	class RowProcessing;
}

//------------------------------------------------------------------------------
class Table: public Object
{
	OBJECT_DECLARE( Table );
public:
	typedef std::vector<Column::Ptr> columns_t;

	unsigned int	ID;
	bool			HasCount; //last column is "Count"
	columns_t Columns;
	llong			TotalCount;
	bool			GroupByApplied; //used by aggregate functions to know when
									//there is a GROUP BY in the query
	bool			OrderByApplied;
	TablePartition::Ptr	Partition;
	bool			NoAnswer;

	Table();
	Table(std::string& name, unsigned int ID );

	int getColumnIdx( const std::string& name );
	
	void loadFromAnswerRaw(	const char *filePath, char fieldSeparator, std::vector<llong>& tableIDs, bool add = false );
	
	void loadFromTableAnswerByRow(aq::AQMatrix& aqMatrix, const std::vector<llong>& tableIDs, const std::vector<Column::Ptr>& columnTypes, const TProjectSettings& pSettings, const Base& BaseDesc, 
																boost::shared_ptr<aq::RowProcessing> rowProcessing);

	void loadFromTableAnswerByColumn(aq::AQMatrix& aqMatrix, const std::vector<llong>& tableIDs, const std::vector<Column::Ptr>& columnTypes, const TProjectSettings& pSettings, const Base& BaseDesc);
  void loadColumn(Column::Ptr col, const std::vector<size_t>& uniqueIndex, const std::vector<size_t>& mapToUniqueIndex, const Column::Ptr columnType, const TProjectSettings& pSettings, const Base& BaseDesc);

	int saveToAnswer(	const char* filePath, char fieldSeparator, 
						std::vector<size_t>& deletedRows, 
						bool answerFormat = true );
	int saveToAnswer( const char* filePath, char fieldSeparator, bool answerFormat = true );
	void cleanRedundantColumns();
	void groupBy();
	void orderBy(	std::vector<Column::Ptr> columns,
					TablePartition::Ptr partition );
	void orderBy(	std::vector<Column::Ptr> columns, 
					TablePartition::Ptr partition,
					std::vector<size_t>& index );
	std::vector<Column::Ptr> getColumnsByName( std::vector<Column::Ptr>& columns );
	void setName( const std::string& name );
	const std::string& getName() const;
	const std::string& getOriginalName() const;
	std::vector<Column::Ptr> getColumnsTemplate();
	void updateColumnsContent( const std::vector<Column::Ptr>& newColumns );
	void unravel( TablePartition::Ptr partition );
  
	void dump( std::ostream& os );
private:
	void computeUniqueRow(Table& aqMatrix, std::vector<std::vector<size_t> >& mapToUniqueIndex, std::vector<std::vector<size_t> >& uniqueIndex) const;

	std::string		Name;
	std::string		OriginalName;
	std::vector<size_t> Index;
	char szBuffer[STR_BUF_SIZE];
};

//------------------------------------------------------------------------------
class VerbResultArray: public VerbResult
{
	OBJECT_DECLARE( VerbResultArray );
public:
	virtual int getType() const { return VerbResult::ARRAY; }

	std::deque<VerbResult::Ptr>	Results;
};

//------------------------------------------------------------------------------
class Base: public Object
{
	OBJECT_DECLARE( Base );
public:
	typedef std::vector<Table> tables_t;
	tables_t Tables;
	std::string Name;

	int getTableIdx( const std::string& name ) const ;

	/// The standard content of this file is defined below :
	/// 
	/// <Name_Base>
	/// <Tbl_count>
	/// 
	/// "<Tbl_Name>" <tbl_id> <tbl_records_nb> <tbl_col_nb>
	/// "<Col_Name>" <col_id> <col_size> <col_type>
	/// 
	/// Example : 
	/// mabase
	/// 4
	/// 
	/// "BONUS" 1 1 4
	/// "ENAME" 1 10 VARCHAR2
	/// "JOB" 2 9 VARCHAR2
	/// "SAL" 3 22 NUMBER
	/// "COMM" 4 22 NUMBER
	///
	/// ...
	///
	void loadFromBaseDesc( const char* pszDataBaseFile );
	void saveToBaseDesc( const char* pszDataBaseFile );
	void dump( std::ostream& os );
};

#endif