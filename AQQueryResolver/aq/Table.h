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

//------------------------------------------------------------------------------
class Base;

//------------------------------------------------------------------------------
class RowProcess_Intf;

//------------------------------------------------------------------------------
class Table: public Object<Table>
{
public:
	typedef std::vector<Column::Ptr> columns_t;

	size_t  	ID;
	bool			HasCount; ///< last column is "Count"
	columns_t Columns;
	uint64_t  TotalCount;
	bool			GroupByApplied; ///< used by aggregate functions to know when there is a GROUP BY in the query
	bool			OrderByApplied;
	bool			NoAnswer;

	Table();
	Table(const std::string& name, unsigned int ID, bool temporary = false);
  Table(const Table& source);
  ~Table();
  Table& operator=(const Table& source);

  void load(const char * path, uint64_t packetSize);
  void loadColumn(Column::Ptr col, const std::vector<size_t>& uniqueIndex, const std::vector<size_t>& mapToUniqueIndex, const Column::Ptr columnType, const Settings& pSettings, const Base& BaseDesc);
  
	int getColumnIdx(const std::string& name)  const;
  Column::Ptr getColumn(const std::string& columnName) const;
	std::vector<Column::Ptr> getColumnsByName(std::vector<Column::Ptr>& columns) const;

	void setName(const std::string& name);
	const std::string& getName() const;
	const std::string& getOriginalName() const;
  
  bool isTemporary() const { return temporary; }
  const char * getTemporaryName() const { return temporaryName.c_str(); }

  void setReferenceTable(const std::string& _referenceTable) { this->referenceTable = _referenceTable; }
  std::string getReferenceTable() const { return this->referenceTable; }

	void dumpRaw(std::ostream& os);
	void dumpXml(std::ostream& os);

private:
	void computeUniqueRow(Table& aqMatrix, std::vector<std::vector<size_t> >& mapToUniqueIndex, std::vector<std::vector<size_t> >& uniqueIndex) const;

	std::string		Name;
	std::string		OriginalName;
  std::string   temporaryName;
  std::string   referenceTable;
	std::vector<size_t> Index;
	char szBuffer[STR_BUF_SIZE];
  bool temporary;
};

}

#endif
