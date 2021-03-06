#include "Optimizations.h"
#include "AuxiliaryVerbs.h"
#include <aq/Exceptions.h>
#include <boost/scoped_array.hpp>

using namespace aq;
using namespace std;
using namespace boost;

//-------------------------------------------------------------------------------
ColumnItem::Ptr getMinMaxFromThesaurus(	int tableIdx, int colIdx, int partIdx, bool min, Base& BaseDesc, TProjectSettings& Settings )
{
	ColumnItem::Ptr minMax = NULL;
	string fileName = getThesaurusFileName( Settings.szThesaurusPath, 
		tableIdx + 1, colIdx + 1, partIdx );
	FILE* pFIn = fopen( fileName.c_str(), "rb" );
	if ( pFIn == NULL )
		return minMax;
	FileCloser fileCloser(pFIn);
	Column::Ptr column = BaseDesc.Tables[tableIdx].Columns[colIdx];

	int binItemSize	= 0;
	int tmpBufSize = 1000;
	switch( column->Type )
	{
	case COL_TYPE_INT: 
		binItemSize = 4;
		break;
	case COL_TYPE_BIG_INT:
	case COL_TYPE_DATE1:
	case COL_TYPE_DATE2:
	case COL_TYPE_DATE3:
	case COL_TYPE_DOUBLE:
		binItemSize = 8;
		break;
	case COL_TYPE_VARCHAR:
		binItemSize = column->Size;
		tmpBufSize = column->Size + 1000;
		break;
	default:
		throw generic_error(generic_error::NOT_IMPLEMENED, "");
	}

	unsigned char *pTmpBuf = new unsigned char[tmpBufSize];
	boost::scoped_array<unsigned char> pTmpBufDel( pTmpBuf );
	if( !min )
	{
		//get the file size
		if( fseek( pFIn, 0, SEEK_END ) != 0 )
			assert( 0 );
		int fileSize = ftell( pFIn );
		if( fileSize == -1 )
			assert( 0 );
		if( fseek( pFIn, fileSize - binItemSize, SEEK_SET ) != 0 )
			assert( 0 );
	}
	if( fread( pTmpBuf, 1, binItemSize, pFIn ) != binItemSize )
		throw generic_error(generic_error::INVALID_FILE, "");

	switch( column->Type )
	{
	case COL_TYPE_INT: 
		{
			int *pItemData = (int*)( pTmpBuf );
			minMax = new ColumnItem( *pItemData );
		}
		break;
	case COL_TYPE_BIG_INT:
	case COL_TYPE_DATE1:
	case COL_TYPE_DATE2:
	case COL_TYPE_DATE3:
		{
			llong *pItemData = (llong*)( pTmpBuf );
			minMax = new ColumnItem( (double) *pItemData );
		}
		break;
	case COL_TYPE_DOUBLE:
		{
			double *pItemData = (double*)( pTmpBuf );
			minMax = new ColumnItem( *pItemData );
		}
		break;
	case COL_TYPE_VARCHAR:
		pTmpBuf[column->Size] = '\0';
		minMax = new ColumnItem( (char*) pTmpBuf );
		break;
	}

	return minMax;
}

//-------------------------------------------------------------------------------
Table::Ptr solveOptimalMinMax(	VerbNode::Ptr spTree, Base& BaseDesc, 
								TProjectSettings& Settings )
{
	if( !spTree->getLeftChild() )
		throw generic_error(generic_error::INVALID_QUERY, "");
	Verb::Ptr verb1 = spTree->getLeftChild()->getVerbObject();
	if( !verb1 || 
		verb1->getVerbType() != K_MIN && 
		verb1->getVerbType() != K_MAX )
		return NULL;
	Verb::Ptr verb2 = spTree->getLeftChild()->getLeftChild()->getVerbObject();
	if( verb2->getVerbType() != K_PERIOD )
		return NULL;
	if( spTree->getBrother() == NULL )
		return NULL;
	VerbNode::Ptr spNode = spTree;
	do
	{
		if( spNode->getVerbObject()->getVerbType() == K_WHERE )
			return NULL;
		spNode = spNode->getBrother();
	}while( spNode->getBrother() );

	ColumnVerb::Ptr columnVerb = dynamic_pointer_cast<ColumnVerb>( verb2 );
	int tableIdx = BaseDesc.getTableIdx( columnVerb->getTableName() );
	int colIdx = BaseDesc.Tables[tableIdx].getColumnIdx( columnVerb->getColumnOnlyName() );
	Column::Ptr column = BaseDesc.Tables[tableIdx].Columns[colIdx];
	ColumnItem::Ptr minMax = NULL;
	bool min = verb1->getVerbType() == K_MIN;
	for( int partIdx = 0; ; ++partIdx )
	{
		ColumnItem::Ptr item = getMinMaxFromThesaurus( tableIdx, 
			colIdx, partIdx, min, BaseDesc, Settings );
		if( !item )
			break;
		if( !minMax )
			minMax = item;
		else
			if( min == lessThan(item.get(), minMax.get(), column->Type) )
				minMax = item;
	}
	
	Table::Ptr table = new Table();
	Column::Ptr newColumn = new Column(*column);
	newColumn->Items.push_back( minMax );
	table->Columns.push_back( newColumn );
	table->TotalCount = 1;
	return table;
}

//-------------------------------------------------------------------------------
bool trivialSelectFromSelect( tnode* pSelect )
{
	assert( pSelect && pSelect->tag == K_SELECT );
	tnode* pFrom = find_main_node( pSelect, K_FROM );
	vector<tnode*> tables;
	commaListToNodeArray( pFrom->left, tables );
	if( tables.size() != 1 )
		return false;
	tnode* pWhere = find_main_node( pSelect, K_WHERE );
	vector<tnode*> conds;
	andListToNodeArray( pWhere->left, conds );
	if( conds.size() == 1 && conds[0]->tag == K_JNO )
		return true;
	else
		return false;
}