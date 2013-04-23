#ifndef __AQ_MATRIX_H__
#define __AQ_MATRIX_H__

#include "Settings.h"
#include "ColumnMapper.h"

#include <aq/DBTypes.h>

#include <map>
#include <vector>
#include <cstdint>

namespace aq
{

class AQMatrix
{
public:
	struct group_by_key_t
	{
		uint8_t * value;
		size_t len;
	};

	struct group_by_key_cmp_t
	{
		bool operator()(const group_by_key_t& k1, const group_by_key_t& k2)
		{
			return (k1.len == k2.len) && (memcmp(k1.value, k2.value, k1.len) < 0);
		}
	};

	typedef std::map<group_by_key_t, std::vector<size_t>, struct group_by_key_cmp_t > group_by_t;

	AQMatrix(const TProjectSettings& settings);
	AQMatrix(const AQMatrix& source);
	~AQMatrix();
	AQMatrix& operator=(const AQMatrix& source);

	// 
	// testing purpose (to remove)
	void simulate(size_t rows, size_t nbTables);

	void load(const char * filePath, const char fieldSeparator, std::vector<long long>& tableIDs);
  
	/// Each column in the table holds a list of row indexes.
	/// Compute a column containing unique and sorted row indexes.
	/// Also compute a mapping between the original row indexes and the sorted and unique indexes
	void computeUniqueRow(std::vector<std::vector<size_t> >& mapToUniqueIndex, std::vector<std::vector<size_t> >& uniqueIndex) const;

	const group_by_t getGroupBy() const { return this->groupByIndex; }

	// void groupBy(const std::map<size_t, std::vector<std::pair<size_t, aq::ColumnType> > >& columnsByTableId);
	void groupBy(std::vector<aq::ColumnMapper::Ptr>& columnsMappers);

	const std::vector<size_t>& getColumn(size_t c) const { return this->matrix[c].indexes; }
	const std::vector<size_t>& getCount() const { return this->count; }
	size_t getNbColumn() const { return this->matrix.size(); }
	uint64_t getTotalCount() const { return this->totalCount; }
	uint64_t getNbRows() const { return this->nbRows; }
	bool hasCountColumn() const { return this->hasCount; }

private:

	struct column_t
	{
		size_t table_id;
		std::vector<size_t> indexes;
		std::vector<size_t> grpKey;
		std::vector<size_t> orderKey;
	};

	typedef std::vector<column_t> matrix_t;

	const TProjectSettings& settings;
	matrix_t matrix;
	std::vector<size_t> count;
	group_by_t groupByIndex;
	uint64_t totalCount;
	uint64_t nbRows;
	bool hasCount;
};

}

#endif