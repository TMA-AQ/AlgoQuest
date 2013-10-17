#ifndef __AQ_BASE_H__
#define __AQ_BASE_H__

#include "Settings.h"
#include "ColumnItem.h"
#include "Column.h"
#include "AQMatrix.h"
#include "ColumnMapper.h"
#include "Table.h"

#include <aq/Object.h>
#include <aq/BaseDesc.h>
#include <aq/DBTypes.h>
#include <aq/Utilities.h>

#include <vector>
#include <string>
#include <deque>

namespace aq
{

//------------------------------------------------------------------------------
class Base: public Object
{
	OBJECT_DECLARE( Base );
public:
	typedef std::vector<Table::Ptr> tables_t;

  tables_t& getTables() { return this->Tables; }
  const tables_t& getTables() const { return this->Tables; }
  const std::string& getName() const { return this->Name; }
  
  Table::Ptr getTable(size_t id);
	Table::Ptr getTable(const std::string& name) ;
  const Table::Ptr getTable(size_t id) const;
	const Table::Ptr getTable(const std::string& name) const;

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
	void dumpRaw( std::ostream& os ) const;
	void dumpXml( std::ostream& os ) const;
  
private:
	tables_t Tables;
	std::string Name;
};

}

#endif