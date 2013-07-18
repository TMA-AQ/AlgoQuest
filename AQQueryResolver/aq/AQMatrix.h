#ifndef __AQ_MATRIX_H__
#define __AQ_MATRIX_H__

#include "Settings.h"
#include "ColumnMapper.h"

#include <aq/DBTypes.h>

#include <string.h>
#include <map>
#include <vector>
#include <cstdint>

namespace aq
{

class Base;

class AQMatrix
{
public:
  typedef std::vector<size_t> v_size_t;

	struct column_t
	{
		size_t table_id;
		v_size_t indexes;
		v_size_t grpKey;
		v_size_t orderKey;
    std::string tableName;
    std::string baseTableName;
	};

	typedef std::vector<column_t> matrix_t;
  typedef std::vector<std::pair<uint64_t, uint64_t> > group_by_t;

	AQMatrix(const TProjectSettings& settings, const Base& baseDesc);
	AQMatrix(const AQMatrix& source);
	~AQMatrix();
	AQMatrix& operator=(const AQMatrix& source);

	// 
	// testing purpose (to remove)
	void simulate(size_t rows, const std::vector<long long>& tableIDs);
  
  void clear();
  void write(const char * filePath);
	void load(const char * filePath, std::vector<long long>& tableIDs);
  void loadHeader(const char * filePath, std::vector<long long>& tableIDs);
  void loadData(const char * filePath);
  void prepareData(const char * filePath);
  void loadNextPacket();
  
	/// Each column in the table holds a list of row indexes.
	/// Compute a column containing unique and sorted row indexes.
	/// Also compute a mapping between the original row indexes and the sorted and unique indexes
	void computeUniqueRow(std::vector<std::vector<size_t> >& mapToUniqueIndex, std::vector<std::vector<size_t> >& uniqueIndex) const;

	const group_by_t& getGroupBy() const { return this->groupByIndex; }

  ///
  void compress();

  ///
  void writeTemporaryTable();

  const matrix_t getMatrix() const { return this->matrix; }
  const size_t getTableId(size_t c) const { return this->matrix[c].table_id; }
	const v_size_t& getColumn(size_t c) const { return this->matrix[c].indexes; }
	const v_size_t& getCount() const { return this->count; }
	size_t getNbColumn() const { return this->matrix.size(); }
	uint64_t getTotalCount() const { return this->totalCount; }
	uint64_t getNbRows() const { return this->nbRows; }
	bool hasCountColumn() const { return this->hasCount; }

  /// Debug purpose
  void dump(std::ostream& os) const;

private:

  static uint64_t uid_generator;
  uint64_t uid;
	const TProjectSettings& settings;
  const Base& baseDesc;
	matrix_t matrix;
	v_size_t count;
  group_by_t groupByIndex;
	uint64_t totalCount;
	uint64_t nbRows;
  std::string answerFormat;
  size_t nbRowsParsed;
  size_t nbPacket;
  size_t packet;
	bool hasCount;
};

}

#endif
