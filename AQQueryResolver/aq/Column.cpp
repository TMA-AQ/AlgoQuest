#include "Column.h"
#include <cstring>
#include <sstream>
#include <aq/Utilities.h>
#include <aq/Exceptions.h>
#include <boost/scoped_array.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>

namespace aq
{

//-----------------------------------------------------------------------------
Column::inner_column_cmp_t::inner_column_cmp_t(Column& lessThanColumn)
	: m_lessThanColumn(lessThanColumn)
{
}
bool Column::inner_column_cmp_t::operator()(size_t idx1, size_t idx2)
{
	return ColumnItem::lessThan(m_lessThanColumn.Items[idx1].get(), m_lessThanColumn.Items[idx2].get(), m_lessThanColumn.Type);
}

Column::inner_column_cmp_2_t::inner_column_cmp_2_t(Column& lessThanColumn)
	: m_lessThanColumn(lessThanColumn)
{
}
bool Column::inner_column_cmp_2_t::operator()(ColumnItem::Ptr item1, ColumnItem::Ptr item2)
{
	return ColumnItem::lessThan(item1.get(), item2.get(), m_lessThanColumn.Type );
}


//------------------------------------------------------------------------------
Column::Column()
	:	
  TableID(0),
	ID(0), 
	Size(0), 
	Type(COL_TYPE_INT), 
	prmFileItemSize(4),
	currentNumPack(-1),
	packOffset(0),
	nBinItemSize(0),
	Invisible(false), 
	GroupBy(false), 
	OrderBy(false),
  Temporary(false)
{
	this->setBinItemSize();
}

//------------------------------------------------------------------------------
Column::Column(	const std::string& name, unsigned int ID, unsigned int size, ColumnType type)
	: 
  TableID(0),
	ID(ID), 
	Size(size), 
	Type(type),
	prmFileItemSize(4),
	currentNumPack(-1),
	packOffset(0),
	nBinItemSize(0),
	Invisible(false), 
	GroupBy(false), 
	OrderBy(false),
  Temporary(false)
{
	this->setBinItemSize();
	this->setName( name );
}

//------------------------------------------------------------------------------
Column::Column( ColumnType type )
	: 
  TableID(0),
	ID(0),
	Size(0),
	Type(type),
	prmFileItemSize(4),
	currentNumPack(-1),
	packOffset(0),
	nBinItemSize(0),
	Invisible(false), 
	GroupBy(false),
	OrderBy(false),
  Temporary(false)
{
	this->setBinItemSize();
}

//------------------------------------------------------------------------------
Column::Column( const Column& source )
	:
  TableID(source.TableID),
	ID(source.ID),
	Size(source.Size),
	Type(source.Type),
	Invisible(false),
	GroupBy(false),
	OrderBy(false),
  Temporary(source.Temporary),
	Name(source.Name),
	OriginalName(source.OriginalName),
	DisplayName(source.DisplayName),
	Count(source.Count),
	TableName(source.TableName),
	prmFileItemSize(source.prmFileItemSize),
	currentNumPack(source.currentNumPack),
	packOffset(source.packOffset),
	nBinItemSize(source.nBinItemSize)
{
	this->setBinItemSize();
}

//----------------------------------------------------------------------------
Column::~Column() 
{
}

//------------------------------------------------------------------------------
Column& Column::operator=(const Column& source)
{
	if (this != &source)
	{
    this->TableID = source.TableID;
		this->ID = source.ID;
		this->Size = source.Size;
		this->Type = source.Type;
		this->Invisible = false;
		this->GroupBy = false;
		this->OrderBy = false;
    this->Temporary = source.Temporary;
		this->Name = source.Name;
		this->OriginalName = source.OriginalName;
		this->DisplayName = source.DisplayName;
		this->Count = source.Count;
		this->TableName = source.TableName;
		this->prmFileItemSize = source.prmFileItemSize;
		this->currentNumPack = source.currentNumPack,
		this->packOffset = source.packOffset;
		this->nBinItemSize = source.nBinItemSize;
		this->setBinItemSize();
	}
	return *this;
}

void Column::setBinItemSize()
{
	switch( this->Type )
	{
	case COL_TYPE_INT:
		nBinItemSize	= 4;
		break;
	case COL_TYPE_BIG_INT:
	case COL_TYPE_DATE:
	case COL_TYPE_DOUBLE:
		nBinItemSize	= 8;
		break;
	case COL_TYPE_VARCHAR:
		nBinItemSize	= this->Size;
		break;
	}
}

//------------------------------------------------------------------------------
void Column::setName( const std::string& name )
{
	this->OriginalName = name;
	this->Name = name;
	boost::to_upper(this->Name);
	boost::trim(this->Name);
}

//------------------------------------------------------------------------------
void Column::setDisplayName( const std::string& name )
{
	this->DisplayName = name;
}

//------------------------------------------------------------------------------
std::string& Column::getName()
{
	return this->Name;
}

//------------------------------------------------------------------------------
std::string& Column::getOriginalName()
{
	return this->OriginalName;
}


//------------------------------------------------------------------------------
std::string& Column::getDisplayName()
{
	if( this->DisplayName == "" )
		if( this->OriginalName == "" )
			return this->Name;
		else
			return this->OriginalName;
	else
		return this->DisplayName;
}

//------------------------------------------------------------------------------
void Column::setTableName( const std::string& name )
{
	this->TableName = name;
	boost::to_upper(this->TableName);
	boost::trim(this->TableName);
}

//------------------------------------------------------------------------------
std::string& Column::getTableName()
{
	return this->TableName;
}

//------------------------------------------------------------------------------
void Column::loadFromTmp(ColumnType eColumnType, aq::TemporaryColumnMapper::Ptr colMapper)
{
  this->Type = eColumnType;
  this->Size = 0;

  int rc = -1;
  do
  {
    ColumnItem::Ptr item(new ColumnItem);
    if ((rc = colMapper->loadValue(this->Size++, *item)) == 0)
      this->Items.push_back(item);
  } while (rc == 0);
  
	inner_column_cmp_2_t cmp(*this);
	sort( this->Items.begin(), this->Items.end(), cmp );

}

//------------------------------------------------------------------------------
void Column::increase( size_t newSize )
{
	assert( this->Items.size() > 0 );
	for( size_t idx = this->Items.size() ; idx < newSize; ++idx )
		this->Items.push_back( this->Items[0] );
}

//------------------------------------------------------------------------------
void Column::setCount( Column::Ptr count )
{
	if( count->Type != COL_TYPE_BIG_INT )
		throw generic_error(generic_error::TYPE_MISMATCH, 
			"Count column set to type other than INT.");
	this->Count = count;
}

//------------------------------------------------------------------------------
Column::Ptr Column::getCount()
{
	return this->Count;
}

//void Column::addItem(size_t index, const Settings& settings, const Base& BaseDesc)
//{
//	size_t numPack = index / ((size_t) settings.packSize);
//	this->packOffset = index % settings.packSize;
//
//	if( numPack != this->currentNumPack )
//	{
//		char szBuffer[128]; // fixme
//		string dataPath = settings.szRootPath;
//		sprintf( szBuffer, "data_orga\\vdg\\data\\B001T%.4uC%.4uV01P%.12u", this->TableID, this->ID, numPack );
//		dataPath += szBuffer;
//
//		string prmFilePath = dataPath + ".prm";
//		string theFilePath = dataPath + ".the";
//
//		aq::Logger::getInstance().log(AQ_DEBUG, "Open prm file %s\n", prmFilePath.c_str());
//		aq::Logger::getInstance().log(AQ_DEBUG, "Open thesaurus %s\n", theFilePath.c_str());
//
//		prmMapper.reset(new aq::FileMapper(prmFilePath.c_str()));
//		thesaurusMapper.reset(new aq::FileMapper(theFilePath.c_str()));
//
//		this->currentNumPack = numPack;
//	}
//
//
//	size_t prmOffset = this->packOffset * this->prmFileItemSize;
//	uint32_t theOffset;
//	prmMapper->read(&theOffset, prmOffset, prmFileItemSize);
//
//	switch( this->Type )
//	{
//	case COL_TYPE_INT:
//		{
//			int32_t *pnItemData;
//			pnItemData = (int*)( this->pTmpBuf );
//			thesaurusMapper->read(this->pTmpBuf, this->nBinItemSize * theOffset, this->nBinItemSize);
//			if( *pnItemData == 'NULL' )
//				this->Items.push_back( NULL );
//			else
//				this->Items.push_back( new ColumnItem( *pnItemData ) );
//		}
//		break;
//	case COL_TYPE_BIG_INT:
//	case COL_TYPE_DATE1:
//	case COL_TYPE_DATE2:
//	case COL_TYPE_DATE3:
//		{
//			int64_t *pnItemData;
//			pnItemData = (long long*)( this->pTmpBuf );
//			thesaurusMapper->read(this->pTmpBuf, this->nBinItemSize * theOffset, this->nBinItemSize);
//			if( *pnItemData == 'NULL' )
//				this->Items.push_back( NULL );
//			else
//				this->Items.push_back( new ColumnItem( (double) *pnItemData ) );
//		}
//		break;
//	case COL_TYPE_DOUBLE:
//		{
//			double *pdItemData;
//			pdItemData = (double*)( this->pTmpBuf );
//			thesaurusMapper->read(this->pTmpBuf, this->nBinItemSize * theOffset, this->nBinItemSize);
//			if( *pdItemData == 'NULL' )
//				this->Items.push_back( NULL );
//			else
//				this->Items.push_back( new ColumnItem( *pdItemData ) );
//		}
//		break;
//	case COL_TYPE_VARCHAR:
//		{	
//			thesaurusMapper->read(this->pTmpBuf, this->nBinItemSize * theOffset, this->nBinItemSize);
//			if( strcmp((char*)this->pTmpBuf, "NULL") == 0 )
//				this->Items.push_back( NULL );
//			else
//				this->Items.push_back( new ColumnItem( string((char*)this->pTmpBuf) ) );
//		}
//		break;
//	}
//}

//------------------------------------------------------------------------------
void Column::dumpRaw( std::ostream& os )
{
  std::string colName = this->getOriginalName();
  size_t pos = colName.find('.');
  if( pos != std::string::npos )
    colName = colName.substr( pos + 1 );
  os << "\""<< colName << "\" " << this->ID << " " << this->Size << " ";
  switch( this->Type )
  {
  case COL_TYPE_INT: os << "INT"; break;
  case COL_TYPE_BIG_INT: os << "BIG_INT"; break;
  case COL_TYPE_DOUBLE: os << "DOUBLE"; break;
  case COL_TYPE_DATE: os << "DATE"; break;
  case COL_TYPE_VARCHAR: os << "VARCHAR2"; break;
  default:
    throw generic_error(generic_error::NOT_IMPLEMENTED, "");
  }
  os << std::endl;
}

//------------------------------------------------------------------------------
void Column::dumpXml( std::ostream& os )
{
  std::string colName = this->getOriginalName();
  size_t pos = colName.find('.');
  if( pos != std::string::npos )
    colName = colName.substr( pos + 1 );
  os << "<Column Name=\"" << colName << "\" ID=\"" << this->ID << "\" Size=\"" << this->Size << "\" Type=\"";
  switch( this->Type )
  {
  case COL_TYPE_INT: os << "INT"; break;
  case COL_TYPE_BIG_INT: os << "BIG_INT"; break;
  case COL_TYPE_DOUBLE: os << "DOUBLE"; break;
  case COL_TYPE_DATE: os << "DATE"; break;
  case COL_TYPE_VARCHAR: os << "VARCHAR2"; break;
  default:
    throw generic_error(generic_error::NOT_IMPLEMENTED, "");
  }
  os << "\"/>" << std::endl;
}

}
