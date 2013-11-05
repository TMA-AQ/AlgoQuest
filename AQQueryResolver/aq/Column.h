#ifndef __AQ_COLUMN_H__
#define __AQ_COLUMN_H__

#include "verbs/VerbResult.h"
#include "Settings.h"
#include "TemporaryColumnMapper.h"

#include <aq/Object.h>
#include <aq/DBTypes.h>
#include <aq/FileMapper.h>

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

namespace aq
{

//------------------------------------------------------------------------------
class Column: public aq::verb::VerbResult
{
public:
  
  typedef boost::intrusive_ptr<Column> Ptr;

	struct inner_column_cmp_t
	{
	public:
		inner_column_cmp_t(Column& lessThanColumn);
		bool operator()(size_t idx1, size_t idx2);
	private:
		Column& m_lessThanColumn;
	};

	struct inner_column_cmp_2_t
	{
	public:
		inner_column_cmp_2_t(Column& lessThanColumn);
		bool operator()(ColumnItem::Ptr item1, ColumnItem::Ptr item2);
	private:
		Column& m_lessThanColumn;
	};

	virtual int getType() const { return VerbResult::COLUMN; }

	Column();
	Column(aq::ColumnType type);
	Column(const std::string& name, unsigned int ID, unsigned int size, aq::ColumnType type);
	Column(const Column& source);
  ~Column();
	Column& operator=(const Column& source);

	void setName( const std::string& name );
	void setDisplayName(const std::string& name);
	std::string& getName();
	std::string& getDisplayName();
	std::string& getOriginalName();

	void setTableName(const std::string& name);
	std::string& getTableName();

  void loadFromTmp(aq::ColumnType eColumnType, aq::TemporaryColumnMapper::Ptr colMapper);
	void increase(size_t newSize);
	void setCount(Column::Ptr count);
	Column::Ptr getCount();
  
	void dumpRaw(std::ostream& os);
	void dumpXml(std::ostream& os);

	std::vector<ColumnItem::Ptr> Items;
	size_t	                     TableID;
	size_t	                     ID;
	size_t	                     Size;	///< maximum size of the text, not number of items
	aq::ColumnType		           Type;
	
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

public:
	bool			Invisible;
	bool			GroupBy;
	bool			OrderBy;
  bool      Temporary;

};

}

#endif