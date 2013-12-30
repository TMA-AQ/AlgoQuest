#ifndef __AQ_BASE_H__
#define __AQ_BASE_H__

#include "Table.h"

#include <vector>
#include <string>
#include <deque>

namespace aq
{

//------------------------------------------------------------------------------
class Base: public Object<Base>
{
public:
	typedef std::vector<Table::Ptr> tables_t;
  
  Base();
  Base(const Base& source);
  Base(const std::string& file);
  ~Base();
  Base& operator=(const Base& source);
  
	/// The standard content of this file is defined below :
	/// 
	/// <Name_Base>
	/// <Tbl_count>
	/// 
	/// "<Tbl_Name>" <tbl_id> <tbl_records_nb> <tbl_col_nb>
	/// "<Col_Name>" <col_id> <col_size> <col_type>
	/// 
	/// Example : 
	/// my_base_name
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
  static int load(const std::string& filename, Base& base);

	void loadFromBaseDesc( const aq::base_t& base );
  void dumpRaw(std::ostream& os) const;
	void dumpXml(std::ostream& os) const;

  void clear();

  tables_t& getTables() { return this->Tables; }
  const tables_t& getTables() const { return this->Tables; }
  const std::string& getName() const { return this->Name; }
  
  Table::Ptr getTable(size_t id);
	Table::Ptr getTable(const std::string& name) ;
  const Table::Ptr getTable(size_t id) const;
	const Table::Ptr getTable(const std::string& name) const;
  
private:
	tables_t    Tables;
	std::string Name;
};

}

#endif