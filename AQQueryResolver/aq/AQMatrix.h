#ifndef __AQ_MATRIX_H__
#define __AQ_MATRIX_H__

#if defined (WIN32)
# ifdef AQENGINE_EXPORTS
#  define AQENGINE_API __declspec(dllexport)
# else
#  define AQENGINE_API __declspec(dllimport)
# endif
#else
# define AQLIB_API __stdcall
#endif

#include "Settings.h"

#include <aq/Base.h>
#include <aq/Exceptions.h>
#include <aq/BaseDesc.h>
#include <aq/DBTypes.h>

#include <string.h>
#include <map>
#include <vector>

#include <boost/scoped_array.hpp>

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

	AQENGINE_API AQMatrix(const Settings& settings, const Base& baseDesc);
	AQENGINE_API AQMatrix(const AQMatrix& source);
	AQENGINE_API ~AQMatrix();
	AQENGINE_API AQMatrix& operator=(const AQMatrix& source);

	// 
	// testing purpose (to remove)
	// void simulate(size_t rows, const std::vector<long long>& tableIDs);
  
  AQENGINE_API void clear();
  AQENGINE_API void setJoinPath(const std::vector<std::string>& jp) { this->joinPath = jp; }
  AQENGINE_API void write(const char * filePath);
	AQENGINE_API void load(const char * filePath, std::vector<long long>& tableIDs);
  AQENGINE_API void loadHeader(const char * filePath, std::vector<long long>& tableIDs);
  AQENGINE_API void loadData(const char * filePath);
  AQENGINE_API void prepareData(const char * filePath);
  AQENGINE_API void loadNextPacket();
  template <class CB> void readData(CB& cb);

	/// Each column in the table holds a list of row indexes.
	/// Compute a column containing unique and sorted row indexes.
	/// Also compute a mapping between the original row indexes and the sorted and unique indexes
	AQENGINE_API void computeUniqueRow(std::vector<std::vector<size_t> >& mapToUniqueIndex, std::vector<std::vector<size_t> >& uniqueIndex) const;

	AQENGINE_API const group_by_t& getGroupBy() const { return this->groupByIndex; }

  ///
  AQENGINE_API void compress();

  ///
  AQENGINE_API void writeTemporaryTable();

  AQENGINE_API const std::vector<std::string>& getJoinPath() { return this->joinPath; }
  AQENGINE_API matrix_t& getMatrix() { return this->matrix; }
  AQENGINE_API const matrix_t& getMatrix() const { return this->matrix; }
  AQENGINE_API const size_t getTableId(size_t c) const { return this->matrix[c].table_id; }
	AQENGINE_API const v_size_t& getColumn(size_t c) const { return this->matrix[c].indexes; }
	AQENGINE_API const v_size_t& getCount() const { return this->count; }
	AQENGINE_API size_t getNbColumn() const { return this->matrix.size(); }
	AQENGINE_API uint64_t getTotalCount() const { return this->totalCount; }
	AQENGINE_API uint64_t getNbRows() const { return this->nbRows; }
	AQENGINE_API bool hasCountColumn() const { return this->hasCount; }

  /// Debug purpose
  AQENGINE_API void dump(std::ostream& os) const;

private:

  static uint64_t uid_generator;
  uint64_t uid;
	const Settings& settings;
  const Base& baseDesc;
	matrix_t matrix;
	v_size_t count;
  group_by_t groupByIndex;
  std::vector<std::string> joinPath;
  uint64_t totalCount;
	uint64_t nbRows;
  std::string answerFormat;
  size_t nbRowsParsed;
  size_t nbPacket;
  size_t packet;
  size_t rowCountCheck;
	bool hasCount;
};

template <class CB>
void AQMatrix::readData(CB& cb)
{
  boost::scoped_array<char> answerData(new char[this->answerFormat.size() + 128]); // FIXME
  sprintf(answerData.get(), this->answerFormat.c_str(), this->packet);
  FILE * fd = fopen(answerData.get(), "rb");
  if (fd == nullptr)
  {
    throw aq::generic_error(aq::generic_error::AQ_ENGINE, "cannot find aq matrix data file %s", answerData.get());
  }
  uint64_t value;
  std::vector<size_t> rows(this->matrix.size() + 1, 0);
  for (size_t i = 0; (i < aq::packet_size) && (i < nbRows); ++i)
  {
    for (size_t c = 0; c < this->matrix.size(); ++c)
    {
      fread(&value, sizeof(uint64_t), 1, fd);
      rows[c] = value;
    }
    fread(&value, sizeof(uint64_t), 1, fd);
    rows[this->matrix.size()] = value;
    cb.handle(rows);
  }
  fclose(fd);
  this->packet += 1;
}

}

#endif
