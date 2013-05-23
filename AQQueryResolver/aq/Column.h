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
class Column: public aq::verb::VerbResult 
{
	OBJECT_DECLARE( Column );
public:

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

	int loadFromThesaurus( const char *pszFilePath, int nFileType, unsigned int nColumnSize, aq::ColumnType eColumnType, int *pErr );
  void loadFromTmp(aq::ColumnType eColumnType, aq::TemporaryColumnMapper::Ptr colMapper);
	void increase( size_t newSize );
	void setCount( Column::Ptr count );
	Column::Ptr getCount();

	// void addItem(size_t index, const TProjectSettings& pSettings, const Base& BaseDesc);

	void loadFromFile( const std::string& file );
	//endIdx == -1 means 'number of items in the column'
	void saveToFile(	const std::string& file, 
						size_t startIdx = 0, size_t endIdx = std::string::npos, 
						bool append = false );
  
	void dumpRaw( std::ostream& os );
	void dumpXml( std::ostream& os );

	std::vector<ColumnItem::Ptr>	Items;
	size_t	TableID;
	size_t	ID;
	size_t	Size;	//maximum size of the text, not number of items
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
  bool      Temporary;

};

#endif