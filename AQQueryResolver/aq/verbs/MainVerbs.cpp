#include "MainVerbs.h"
#include "VerbVisitor.h"
#include <algorithm>
#include <aq/parser/ID2Str.h>
#include <aq/Exceptions.h>
#include <aq/TreeUtilities.h>
#include <aq/Column2Table.h>
#include <aq/Logger.h>

using namespace aq;
using namespace std;

//------------------------------------------------------------------------------
VERB_IMPLEMENT(SelectVerb);

//------------------------------------------------------------------------------
SelectVerb::SelectVerb()
{
}

//------------------------------------------------------------------------------
void getAllColumns( aq::tnode* pNode, vector<aq::tnode*>& columns )
{
	if( !pNode || pNode->inf == 1 && pNode->tag != K_COMMA || pNode->tag == K_JNO )
		return;
	if( pNode->tag == K_PERIOD )
	{
		//only if the column name is unique
		bool found = false;
		for( size_t idx = 0; idx < columns.size(); ++idx )
		{
			if( !columns[idx] )
				continue;

			std::string table1(columns[idx]->left->data.val_str);
			strtoupr( table1 );
			std::string table2(pNode->left->data.val_str);
			strtoupr( table2 );
			std::string col1(columns[idx]->right->data.val_str);
			strtoupr( col1 );
			std::string col2(pNode->right->data.val_str);
			strtoupr( col2 );

			if( columns[idx]->tag == K_PERIOD &&
				table1 == table2 && col1 == col2 )
			{
				found = true;
				break;
			}
		}
		if( !found )
		{
			columns.push_back( clone_subtree(pNode) );
			pNode = NULL;
		}
		return;
	}
	getAllColumns( pNode->left, columns );
	getAllColumns( pNode->right, columns );
	getAllColumns( pNode->next, columns );
}

//------------------------------------------------------------------------------
void extractName( aq::tnode* pNode, std::string& name )
{
	if( !pNode )
		return;
	if( pNode->tag == K_AS )
	{
		name += pNode->right->data.val_str;
	}
	else if( pNode->tag == K_PERIOD )
	{
		if( name != "" )
			name += " ";
		name += pNode->left->data.val_str;
		name += ".";
		name += pNode->right->data.val_str;
	}
	else if( pNode->tag == K_COLUMN )
	{
		name += pNode->data.val_str;
	}
	else
	{
		std::string idstr = std::string( id_to_string( pNode->tag ) );
		if( idstr != "" )
		{
			if( name != "" )
				name += " ";
			name += idstr;
		}
		extractName( pNode->left, name );
		extractName( pNode->right, name );
	}
}

//------------------------------------------------------------------------------
bool SelectVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	assert( pStartOriginal->tag == pNode->tag );
	vector<aq::tnode*> columns;
	columns.clear();
	getColumnsList( pStartOriginal->left, columns );
	for( size_t idx = 0; idx < columns.size(); ++idx )
	{
		std::string name;
		extractName( columns[idx], name );
		this->ColumnsDisplay.push_back( name );
	}
	return false;
}

//------------------------------------------------------------------------------
bool SelectVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{

	if( resLeft && resLeft->getType() == VerbResult::ASTERISK )
	{
		this->Columns.clear();
		solveSelectStar( pNode, *this->m_baseDesc, this->Columns, this->ColumnsDisplay );
		return false;
	}
	vector<aq::tnode*> columns;
	getColumnsList( pNode->left, columns );
	for( size_t idx = 0; idx < columns.size(); ++idx )
	{
		std::string name;
		if( columns[idx]->tag == K_PERIOD )
		{
			name += columns[idx]->left->data.val_str;
			name += ".";
			name += columns[idx]->right->data.val_str;
		}
		else
		{
			if( columns[idx]->tag == K_AS && columns[idx]->left->tag == K_PERIOD )
			{
				name += columns[idx]->left->left->data.val_str;
				name += ".";
				name += columns[idx]->left->right->data.val_str;
			}
			columns[idx]->tag = K_DELETED;
			columns[idx] = NULL;
		}
		this->Columns.push_back( name );
	}

	assert( this->Columns.size() == this->ColumnsDisplay.size() );
	getAllColumns( pNode, columns );

	//add extra columns
	if( this->Columns.size() == columns.size() )
		return false; //no extra columns
		
	if( pNode->left->tag == K_COMMA )
	{
		pNode = pNode->left;
		while( pNode->left && pNode->left->tag == K_COMMA )
			pNode = pNode->left;
	}
	
	aq::tnode* pAuxNode = pNode->left;
	pNode->left = new_node( K_COMMA );
	pNode = pNode->left;
	pNode->right = pAuxNode;
	
	for( size_t idx = this->Columns.size(); idx < columns.size() - 1; ++idx )
	{
		pNode->left = new_node( K_COMMA );
		pNode = pNode->left;
		pNode->right = columns[idx];
	}
	pNode->left = columns[columns.size() - 1];

	return false;
}

//------------------------------------------------------------------------------
void SelectVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft,
								VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	assert( resLeft && !resRight );
	if( resLeft->getType() == VerbResult::ASTERISK )
	{
		assert( this->Columns.size() <= table->Columns.size() );
		for( size_t idx = 0; idx < this->Columns.size(); ++idx )
			table->Columns[idx]->setName( this->Columns[idx] );
		return;
	}
	//either one column or one K_COMMA operator
	VerbResultArray::Ptr resArray = dynamic_pointer_cast<VerbResultArray>( resLeft );
	if( !resArray )
	{
		resArray = new VerbResultArray();
		resArray->Results.push_back( resLeft );
	}

	std::vector<Column::Ptr> newColumns;
	size_t tableIdx = 0;
	std::deque<VerbResult::Ptr>& params = resArray->Results;
	assert( params.size() == this->Columns.size() );
	Column::Ptr foundColumn = NULL;
	size_t nrColumns = table->Columns.size();
	if( table->HasCount )
		--nrColumns;
	for( size_t idx = 0; idx < nrColumns; ++idx )
		table->Columns[idx]->Invisible = true;
	for( size_t idx = 0; idx < this->Columns.size(); ++idx )
	{
		if( !params[idx] )
			throw generic_error(generic_error::GENERIC, "");
		Column::Ptr column;
		if( params[idx]->getType() == VerbResult::COLUMN )
		{
			column = static_pointer_cast<Column>( params[idx] );
			newColumns.push_back( column );
			if( !foundColumn )
				foundColumn = column;
		}
		else
		{
			assert( params[idx]->getType() == VerbResult::SCALAR );
			Scalar::Ptr scalar = static_pointer_cast<Scalar>( params[idx] );
			column = new Column(scalar->Type);
			column->Items.push_back(new ColumnItem(scalar->Item));
			newColumns.push_back( column );
		}
		if( idx < this->Columns.size() )
		{
			column->setName( this->Columns[idx] );
			column->setDisplayName( this->ColumnsDisplay[idx] );
		}
		column->Invisible = false;
	}

	//add the rest of the columns
	for( size_t idx = 0; idx < table->Columns.size(); ++idx )
		if( table->Columns[idx]->Invisible )
			newColumns.push_back( table->Columns[idx] );
	/*{
		bool found = false;
		for( size_t idx2 = 0; idx2 < newColumns.size(); ++idx2 )
			if( newColumns[idx2]->getName() == table->Columns[idx]->getName() )
			{
				found = true;
				break;
			}
			if( !found )
			{
				table->Columns[idx]->Invisible = true;
				newColumns.push_back( table->Columns[idx] );
			}
	}*/

	if( table->HasCount && foundColumn )
		newColumns.push_back( table->Columns[table->Columns.size()-1] );
	if( table->HasCount && !foundColumn )
		table->HasCount = false;

	//turn scalars into columns
	if( foundColumn )
		for( size_t idx = 0; idx < newColumns.size(); ++idx )
			if( newColumns[idx]->Items.size() < foundColumn->Items.size() )
				newColumns[idx]->increase( foundColumn->Items.size() );

	if( !foundColumn )
		table->TotalCount = 1;

	table->Columns = newColumns;
}

void SelectVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( WhereVerb );

//------------------------------------------------------------------------------
WhereVerb::WhereVerb()
{}

//------------------------------------------------------------------------------
//traverse subtree recursively and negate operators when needed
//if there is a NOT to be applied and the current node is
//NOT: delete this node and make the child node this node
//     if no NOT applies, apply NOT to child node
//     if a NOT already applies, do not apply NOT to child node
//<, <=, >, >=, =, <>, BETWEEN, LIKE : change into the inverse version
//OR : change to AND, apply NOT on children
//AND: change to OR, apply NOT on children
void processNot( aq::tnode*& pNode, bool applyNot )
{
	if( !pNode )
		return;
	switch( pNode->tag )
	{
	case K_NOT:
		{
			aq::tnode* auxNode = pNode;
			pNode = pNode->left;
			auxNode->left = NULL;
			delete_node( auxNode );
			processNot( pNode, !applyNot );
		}
		break;
	case K_AND:
		{
			if( applyNot )
				pNode->tag = K_OR;
			processNot( pNode->left, applyNot );
			processNot( pNode->right, applyNot );
		}
		break;
	case K_OR:
		{
			if( applyNot )
				pNode->tag = K_AND;
			processNot( pNode->left, applyNot );
			processNot( pNode->right, applyNot );
		}
		break;
	case K_LT:
		if( applyNot )
			pNode->tag = K_GEQ;
		break;
	case K_LEQ:
		if( applyNot )
			pNode->tag = K_GT;
		break;
	case K_GT:
		if( applyNot )
			pNode->tag = K_LEQ;
		break;
	case K_GEQ:
		if( applyNot )
			pNode->tag = K_LT;
		break;
	case K_JSEQ:
		if( applyNot )
			pNode->tag = K_JINF;
		break;
	case K_JSUP:
		if( applyNot )
			pNode->tag = K_JIEQ;
		break;
	case K_JINF:
		if( applyNot )
			pNode->tag = K_JSEQ;
		break;
	case K_JIEQ:
		if( applyNot )
			pNode->tag = K_JSUP;
		break;
	case K_JEQ:
		if( applyNot )
			pNode->tag = K_JNEQ;
		break;
	case K_JAUTO:
		if( applyNot )
			pNode->tag = K_JAUTO;
		break;
	case K_JNEQ:
		if( applyNot )
			pNode->tag = K_JEQ;
		break;
	case K_EQ:
		if( applyNot )
			pNode->tag = K_NEQ;
		break;
	case K_NEQ:
		if( applyNot )
			pNode->tag = K_EQ;
		break;
	case K_BETWEEN:
		if( applyNot )
			pNode->tag = K_NOT_BETWEEN;
		break;
	case K_NOT_BETWEEN:
		if( applyNot )
			pNode->tag = K_BETWEEN;
		break;
	case K_LIKE:
		if( applyNot )
			pNode->tag = K_NOT_LIKE;
		break;
	case K_NOT_LIKE:
		if( applyNot )
			pNode->tag = K_LIKE;
		break;
	case K_IN:
		if( applyNot )
			pNode->tag = K_NOT_IN;
		break;
	case K_NOT_IN:
		if( applyNot )
			pNode->tag = K_IN;
		break;
	case K_JNO:
		break;
	case K_IS:
		if( !pNode->right )
			throw generic_error(generic_error::NOT_IMPLEMENED, "");
		if( pNode->right->tag == K_NOT && 
			pNode->right->left && pNode->right->left->tag == K_NULL )
		{
			aq::tnode* auxNode = pNode->right;
			pNode->right = pNode->right->left;
			delete_node( auxNode );
		}
		else if( pNode->right->tag == K_NULL )
		{
			aq::tnode* auxNode = new_node( K_NOT );
			auxNode->left = pNode->right;
			pNode->right = auxNode;
		}
		else throw generic_error(generic_error::NOT_IMPLEMENED, "");
		break;
	default:
		throw generic_error(generic_error::NOT_IMPLEMENED, "");
	}	
}

//------------------------------------------------------------------------------
bool WhereVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	//eliminate K_NOT
	processNot( pNode->left, false );
	
	return false;
}

//------------------------------------------------------------------------------
bool WhereVerb::changeQuery( aq::tnode* pStart, aq::tnode* pNode,
	VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	addInnerOuterNodes( pNode->left, K_INNER, K_INNER );
	return false;
}

//------------------------------------------------------------------------------
void WhereVerb::changeResult( Table::Ptr table, 
	VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	if( !resLeft )
		return;

	if( resLeft->getType() != VerbResult::ROW_VALIDATION )
		throw verb_error(verb_error::VERB_BAD_SYNTAX, this->getVerbType());
	RowValidation::Ptr rv = dynamic_pointer_cast<RowValidation>( resLeft );
	
	//recreate table
	vector<Column::Ptr> newColumns = table->getColumnsTemplate();

	for( size_t idx = 0; idx < rv->ValidRows.size(); ++idx )
	{
		if( !rv->ValidRows[idx] )
			continue;
		for( size_t idx2 = 0; idx2 < table->Columns.size(); ++idx2 )
			newColumns[idx2]->Items.push_back( table->Columns[idx2]->Items[idx] );
	}

	table->updateColumnsContent( newColumns );
}

void WhereVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( OrderVerb );

//------------------------------------------------------------------------------
OrderVerb::OrderVerb()
{}

/*//------------------------------------------------------------------------------
void replaceColumnAlias( aq::tnode* pNode, const char* alias, aq::tnode* column )
{
	if( !pNode )
		return;
	if( pNode->tag == K_IDENT && strcmp(alias, pNode->data.val_str) == 0 )
	{
		pNode->tag = K_PERIOD;
		set_int_data( pNode, 0 );
		pNode->left = new_node( K_IDENT );
		set_string_data( pNode->left, column->left->data.val_str );
		pNode->right = new_node( K_COLUMN );
		set_string_data( pNode->right, column->right->data.val_str );
	}

	replaceColumnAlias( pNode->left, alias, column );
	replaceColumnAlias( pNode->right, alias, column );
	replaceColumnAlias( pNode->next, alias, column );
}*/

//------------------------------------------------------------------------------
bool OrderVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	vector<aq::tnode*> columns;
	getColumnsList( pNode->left->left, columns );
	vector<aq::tnode*> selectColumns;
	getColumnsList( pStart->left, selectColumns );
	for( size_t idx = 0; idx < columns.size(); ++idx )
		if( columns[idx]->tag == K_IDENT || 
			columns[idx]->tag == K_COLUMN )
		{
			int colIdx = -1;
			for( size_t idx2 = 0; idx2 < selectColumns.size(); ++idx2 )
				if( selectColumns[idx2] &&
					selectColumns[idx2]->tag == K_AS &&
					strcmp( selectColumns[idx2]->right->data.val_str,
						columns[idx]->data.val_str ) == 0
					)
				{
					colIdx = (int) idx2;
					break;
				}
			if( colIdx < 0 )
				throw verb_error(generic_error::INVALID_QUERY, this->getVerbType());
			columns[idx]->tag = K_INTEGER;
			set_int_data( columns[idx], colIdx + 1 );
		}
	return false;
}

//------------------------------------------------------------------------------
bool OrderVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
								VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	pNode->tag = K_DELETED;
	return false;
}

//------------------------------------------------------------------------------
void OrderVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	assert( resLeft );
	VerbResultArray::Ptr resArray = dynamic_pointer_cast<VerbResultArray>( resLeft );
	if( !resArray )
	{
		resArray = new VerbResultArray();
		resArray->Results.push_back( resLeft );
	}
	assert( resArray->Results.size() > 0 );
	vector<Column::Ptr> selectColumns;
	for( size_t idx = 0; idx < table->Columns.size(); ++idx )
		if( !table->Columns[idx]->Invisible )
			selectColumns.push_back( table->Columns[idx] );

	vector<Column::Ptr> columns;
	for( size_t idx = 0; idx < resArray->Results.size(); ++idx )
	{
		
		VerbResult::Ptr result = resArray->Results[idx];
		assert( result );
		switch( result->getType() )
		{
		case VerbResult::COLUMN:
			columns.push_back( static_pointer_cast<Column>( result ) );
			break;
		case VerbResult::SCALAR:
			{
				Scalar::Ptr scalar = static_pointer_cast<Scalar>( result );
				if( scalar->Type != COL_TYPE_INT &&
					scalar->Type != COL_TYPE_BIG_INT )
					throw verb_error(generic_error::INVALID_QUERY, this->getVerbType());
				int colNum = (int) scalar->Item.numval;
				--colNum;
				if( colNum < 0 || colNum >= (int) selectColumns.size() )
					throw verb_error(generic_error::INVALID_QUERY, this->getVerbType());
				columns.push_back( selectColumns[colNum] );
			}
			break;
		default:
			throw verb_error(generic_error::INVALID_QUERY, this->getVerbType());
		}
	}

	TablePartition::Ptr partition = NULL;
	if( this->Context == K_SELECT )
	{
		if( resRight )
		{
			assert( resRight->getType() == VerbResult::TABLE_PARTITION );
			partition = static_pointer_cast<TablePartition>( resRight );
		}
	}
	
	if( this->Context != K_SELECT )
	{
		//we do a final group by at the end of the query, but it should not interfere
		//with the results of a final order by, so do the group by before
		table->cleanRedundantColumns();
		table->groupBy();
		table->OrderByApplied = true;
	}
	//make copy so that the partition returned by PARTITION BY is not changed
	if( !partition )
	{
		partition = new TablePartition();
		partition->FrameEndType = TablePartition::AQ_RELATIVE;
		partition->Rows.push_back( 0 );
		if( table->Columns.size() > 0 )
			partition->Rows.push_back( (int) table->Columns[0]->Items.size() );
		else
			partition->Rows.push_back( 0 );
	}
	else
		if( !partition->FrameUnitsInitialized )
			partition->FrameEndType = TablePartition::AQ_RELATIVE;

	TablePartition::Ptr partitionCopy = new TablePartition( *partition );
	table->orderBy( columns, partitionCopy );
	
	if( this->Context == K_SELECT )
		this->Result = partition;
}

//------------------------------------------------------------------------------
void OrderVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( ByVerb );

//------------------------------------------------------------------------------
ByVerb::ByVerb()
{}

//------------------------------------------------------------------------------
bool ByVerb::changeQuery( aq::tnode* pStart, aq::tnode* pNode,
							VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	return false;
}

//------------------------------------------------------------------------------
void ByVerb::changeResult(	Table::Ptr table, 
							VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	//simply move the parameters up to the parent
	assert( resLeft && !resRight );
	this->Result = resLeft;
}

//------------------------------------------------------------------------------
void ByVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( FromVerb );

//------------------------------------------------------------------------------
FromVerb::FromVerb()
{}
/*
//------------------------------------------------------------------------------
void replaceTableIdent( aq::tnode* pNode, const char* oldIdent, const char* newIdent )
{
	if( !pNode )
		return;
	if( pNode->tag == K_PERIOD && strcmp(oldIdent, pNode->left->data.val_str) == 0 )
		set_string_data( pNode->left, newIdent );
		
	replaceTableIdent( pNode->left, oldIdent, newIdent );
	replaceTableIdent( pNode->right, oldIdent, newIdent );
	//avoid changing column names in the main ORDER BY because it uses the 
	//names/aliases given to columns after SELECT is done executing
	//debug13 - this is a debug13 because columns that are not in SELECT
	//can appear in ORDER BY, which ruins my initial solution for this problem
	if( pNode->next && pNode->next->tag != K_ORDER ) 
		replaceTableIdent( pNode->next, oldIdent, newIdent );
}
*/

//------------------------------------------------------------------------------
void PreProcessSelect( aq::tnode *pNode, Base& BaseDesc )
{
	int				nRet;
	// Corect Column References !
	TColumn2TablesArray* parrC2T;
	parrC2T = create_column_map_for_tables_used_in_select( pNode, &BaseDesc );

	if ( parrC2T != NULL ) {
		nRet = 0;	/* Required by "enforce_qualified_column_reference()" */
		enforce_qualified_column_reference( pNode, parrC2T, &nRet );
		delete_column2tables_array( parrC2T );
		if ( nRet == 0 ) {
			//pNode = expression_transform( pNode, &BaseDesc, pSettings->szThesaurusPath, &nRet );
			if ( nRet == 0 && pNode != NULL ) {
			} else {
				aq::Logger::getInstance().log(AQ_ERROR, "Function expression_transform() returned error : %d !\n", nRet);
			}
		} else {
			aq::Logger::getInstance().log(AQ_ERROR, "Function enforce_qualified_column_reference() returned error : %d !\n", nRet);
		}
	} else {
		aq::Logger::getInstance().log(AQ_ERROR, "Function create_column_map_for_tables_used_in_select() returned NULL !\n");
		throw generic_error(generic_error::INVALID_QUERY, "Error : No or bad tables specified in SQL SELECT ... FROM Statement");
	}
}

//------------------------------------------------------------------------------
bool FromVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	PreProcessSelect( pStart, *this->m_baseDesc );
  getTablesList(pNode, this->tables);
	return false;
}

//------------------------------------------------------------------------------
bool FromVerb::changeQuery( aq::tnode* pStart, aq::tnode* pNode,
	VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	solveOneTableInFrom( pStart, *this->m_baseDesc );
	moveFromJoinToWhere( pStart, *this->m_baseDesc );
	return false;
}

//------------------------------------------------------------------------------
void FromVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( GroupVerb );

//------------------------------------------------------------------------------
GroupVerb::GroupVerb()
{}

//------------------------------------------------------------------------------
bool GroupVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
  return this->useRowResolver;
}

//------------------------------------------------------------------------------
bool GroupVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
	pNode->tag = K_DELETED;
	return false;
}

//------------------------------------------------------------------------------
void GroupVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
	assert( resLeft );
	assert( resLeft->getType() == VerbResult::ARRAY ||
			resLeft->getType() == VerbResult::COLUMN );
	
	vector<Column::Ptr> columns;
	if( resLeft->getType() == VerbResult::ARRAY )
	{
		VerbResultArray::Ptr resArray = static_pointer_cast<VerbResultArray>( resLeft );
		for( size_t idx = 0; idx < resArray->Results.size(); ++idx )
			columns.push_back( static_pointer_cast<Column>( resArray->Results[idx] ) );
	}
	else
		columns.push_back( static_pointer_cast<Column>( resLeft ) );

	table->groupBy();
	table->Partition = new TablePartition();
	table->orderBy( columns, table->Partition );
	table->GroupByApplied = true;
	/*

	Column::Ptr foundColumn = NULL;
	//get extra columns needed for group by
	Column::Ptr count = NULL;
	if( table->HasCount )
	{
		count = table->Columns[table->Columns.size() - 1];
		table->Columns.pop_back();
	}
	for( size_t idx = 0; idx < columns.size(); ++idx )
	{
		bool found = false;
		for( size_t idx2 = 0; idx2 < table->Columns.size(); ++idx2 )
			if( table->Columns[idx2]->getName() == columns[idx]->getName() )
			{
				table
				break;
			}
			if( !found )
			{
				if( !foundColumn )
					foundColumn = columns[idx];
				columns[idx]->Invisible = true;
				table->Columns.push_back( columns[idx] );
			}
	}
	if( table->HasCount )
		table->Columns.push_back( count );

	//make sure to grow ex-scalars if needed
	if( foundColumn )
		for( size_t idx = 0; idx < table->Columns.size(); ++idx )
			if( table->Columns[idx]->Items.size() < foundColumn->Items.size() )
				table->Columns[idx]->increase( foundColumn->Items.size() );
	*/
}

//------------------------------------------------------------------------------
void GroupVerb::addResult(aq::RowProcess_Intf::Row& row, 
                          VerbResult::Ptr resLeft, 
                          VerbResult::Ptr resRight, 
                          VerbResult::Ptr resNext )
{
  assert((this->row_acc.row.size() == 0) || (row.flush) || (row.row.size() == this->row_acc.row.size()));
  assert((this->row_prv.row.size() == 0) || (row.flush) || (row.row.size() == this->row_prv.row.size()));
  assert(this->row_prv.row.size() == this->row_acc.row.size());
  
  row.completed = false;

  // flush
  if (row.flush)
  {
    row.completed = true;
    row.row.clear();
    std::copy(this->row_acc.row.begin(), this->row_acc.row.end(), std::back_inserter<aq::RowProcess_Intf::row_t>(row.row));
    return;
  }

  // check if new group
  bool new_group = false;
  if (row_prv.row.size() > 0)
  {
    for (size_t i = 0; i < row.row.size(); ++i)
    {
      if (row.row[i].grouped && !ColumnItem::equal(row_prv.row[i].item.get(), row.row[i].item.get(), row_prv.row[i].type))
      {
        new_group = true;
      }
    }
  }
  row.completed = new_group;

  // store prv
  if (this->row_prv.row.size() == 0)
  {
    row.completed = false;
    std::copy(row.row.begin(), row.row.end(), std::back_inserter<aq::RowProcess_Intf::row_t>(this->row_prv.row));
  }

  row_prv.row.clear();
  std::copy(row.row.begin(), row.row.end(), std::back_inserter<aq::RowProcess_Intf::row_t>(this->row_prv.row));

  if (new_group)
  {
    row.row.clear();
    std::copy(this->row_acc.row.begin(), this->row_acc.row.end(), std::back_inserter<aq::RowProcess_Intf::row_t>(row.row));
    this->row_acc.row.clear();
  }
   
  // compute and store in row_acc
  double count = this->row_prv.row[0].item->numval;
  if (this->row_acc.row.empty())
  {
    std::copy(this->row_prv.row.begin(), this->row_prv.row.end(), std::back_inserter<aq::RowProcess_Intf::row_t>(this->row_acc.row));
    for (size_t i = 1; i < row.row.size(); ++i)
    {
      if (row_acc.row[i].item == NULL)
      {
        row_acc.row[i].item = row_prv.row[i].item;
        switch (row.row[i].aggFunc)
        {
        case SUM:
          row_acc.row[i].item->numval *= count;
          break;
        default:
          break;
        }
      }
    }
  }
  else
  {
    for (size_t i = 1; i < row.row.size(); ++i)
    {
      switch (row.row[i].aggFunc)
      {
      case MIN:
        row_acc.row[i].item->numval = (std::min)(row_acc.row[i].item->numval, row_prv.row[i].item->numval);
        break;
      case MAX:
        row_acc.row[i].item->numval = (std::max)(row_acc.row[i].item->numval, row_prv.row[i].item->numval);
        break;
      case SUM:
        row_acc.row[i].item->numval += (count * row_prv.row[i].item->numval);
        break;
      case AVG:
        row_acc.row[i].item->numval = ((row_acc.row[0].item->numval * row_acc.row[i].item->numval) + (count * row_prv.row[i].item->numval)) / (row_acc.row[0].item->numval + count);
        break;
      case COUNT:
        break;
      }
    }
    this->row_acc.row[0].item->numval += count;
  }

}

//------------------------------------------------------------------------------
void GroupVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( HavingVerb );

//------------------------------------------------------------------------------
HavingVerb::HavingVerb()
{}

//------------------------------------------------------------------------------
bool HavingVerb::preprocessQuery(	aq::tnode* pStart, aq::tnode* pNode, 
									aq::tnode* pStartOriginal )
{
	//eliminate K_NOT
	processNot( pNode->left, false );

	return false;
}

//------------------------------------------------------------------------------
void HavingVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
	if( !resLeft )
		return;

	if( resLeft->getType() != VerbResult::ROW_VALIDATION )
		throw verb_error(verb_error::VERB_BAD_SYNTAX, this->getVerbType());
	RowValidation::Ptr rv = dynamic_pointer_cast<RowValidation>( resLeft );

	//recreate table
	vector<Column::Ptr> newColumns = table->getColumnsTemplate();

	for( size_t idx = 0; idx < rv->ValidRows.size(); ++idx )
	{
		if( !rv->ValidRows[idx] )
			continue;
		for( size_t idx2 = 0; idx2 < table->Columns.size(); ++idx2 )
			newColumns[idx2]->Items.push_back( table->Columns[idx2]->Items[idx] );
	}

	table->updateColumnsContent( newColumns );

	//update group by partitions
	std::vector<size_t>& oldRows = table->Partition->Rows;
	assert( oldRows.size() > 0 );
	std::vector<size_t> rows;
	size_t skipped = 0;
	for( size_t idx = 0; idx < oldRows.size() - 1; ++idx )
		if( !rv->ValidRows[oldRows[idx]] )
		{
			skipped += oldRows[idx+1] - oldRows[idx];
		}
		else
		{
			size_t last = oldRows[idx] - skipped;
			if( rows.size() == 0 || rows[rows.size() - 1] != last )
				rows.push_back( last );
			rows.push_back( oldRows[idx + 1] - skipped );
		}
	if( rows.size() == 0 )
	{
		rows.push_back( 0 );
		rows.push_back( 0 );
	}
	table->Partition->Rows = rows;
}

//------------------------------------------------------------------------------
void HavingVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}
