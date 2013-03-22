#ifndef __AQ_MATRIX_H__
#define __AQ_MATRIX_H__

#include <map>
#include <vector>
#include <cstdint>

namespace aq
{

class AQMatrix
{
public:
	AQMatrix();
	AQMatrix(const AQMatrix& source);
	~AQMatrix();
	AQMatrix& operator=(const AQMatrix& source);

	// 
	// testing purpose (to remove)
	void simulate(size_t rows, size_t nbTables);

	void load(const char * filePath, const char fieldSeparator, std::vector<long long>& tableIDs);
	void computeUniqueRow(std::vector<std::vector<size_t> >& mapToUniqueIndex, std::vector<std::vector<size_t> >& uniqueIndex) const;
	const std::vector<size_t> getGroupBy() const { return this->groupByIndex; }

	void groupBy(const std::map<size_t, std::vector<size_t> >& columnsByTableId);

	const std::vector<uint64_t>& getColumn(size_t c) const { return this->indexes[c]; }
	size_t getNbColumn() const { return this->indexes.size(); }
	uint64_t getTotalCount() const { return this->totalCount; }
	uint64_t getNbRows() const { return this->nbRows; }
	bool hasCountColumn() const { return this->hasCount; }

private:
	std::vector<std::vector<uint64_t> > indexes;
	std::vector<size_t> groupByIndex;
	uint64_t totalCount;
	uint64_t nbRows;
	bool hasCount;
};

}

#endif