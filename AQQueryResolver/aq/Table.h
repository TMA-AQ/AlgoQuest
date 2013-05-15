#ifndef __AQ_TABLE_H__
#define __AQ_TABLE_H__

#include "Settings.h"
#include "ColumnItem.h"
#include "Column.h"
#include "AQMatrix.h"
#include "ColumnMapper.h"

#include <aq/Object.h>
#include <aq/BaseDesc.h>
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
namespace aq
{
	class RowProcess_Intf;
}

//------------------------------------------------------------------------------
class Table: public Object
{
	OBJECT_DECLARE( Table );
public:
	typedef std::vector<Column::Ptr> columns_t;

	size_t  	ID;
	bool			HasCount; //last column is "Count"
	columns_t Columns;
	uint64_t  TotalCount;
	bool			GroupByApplied; //used by aggregate functions to know when
									//there is a GROUP BY in the query
	bool			OrderByApplied;
	TablePartition::Ptr	Partition;
	bool			NoAnswer;

	Table();
	Table(const std::string& name, unsigned int ID, bool temporary = false );

	int getColumnIdx( const std::string& name );
	
	void loadFromAnswerRaw(	const char *filePath, char fieldSeparator, std::vector<llong>& tableIDs, bool add = false );
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
  
  bool isTemporary() const { return temporary; }
  const char * getTemporaryName() const { return temporaryName.c_str(); }

	void dumpRaw( std::ostream& os );
	void dumpXml( std::ostream& os );

private:
	void computeUniqueRow(Table& aqMatrix, std::vector<std::vector<size_t> >& mapToUniqueIndex, std::vector<std::vector<size_t> >& uniqueIndex) const;

	std::string		Name;
	std::string		OriginalName;
  std::string   temporaryName;
	std::vector<size_t> Index;
	char szBuffer[STR_BUF_SIZE];
  bool temporary;
};

//------------------------------------------------------------------------------
class Base: public Object
{
	OBJECT_DECLARE( Base );
public:
	typedef std::vector<Table::Ptr> tables_t;
	tables_t Tables;
	std::string Name;

	size_t getTableIdx( const std::string& name ) const ;

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
	void loadFromBaseDesc( const aq::base_t& base );
	void loadFromRawFile( const char* pszDataBaseFile );
	void saveToRawFile( const char* pszDataBaseFile );
	void dumpRaw( std::ostream& os );
	void dumpXml( std::ostream& os );
};

#endif