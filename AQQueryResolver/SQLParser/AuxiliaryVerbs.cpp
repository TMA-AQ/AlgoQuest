#include "AuxiliaryVerbs.h"
#include <aq/Utilities.h>
#include "ExprTransform.h"
#include <aq/Exceptions.h>
#include "VerbVisitor.h"
#include <algorithm>

using namespace aq;
using namespace std;
using namespace boost;

//------------------------------------------------------------------------------
VERB_IMPLEMENT( ColumnVerb );

//------------------------------------------------------------------------------
ColumnVerb::ColumnVerb()
{}

//------------------------------------------------------------------------------
bool ColumnVerb::changeQuery(	tnode* pStart, tnode* pNode,
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	assert( pNode->left->tag == K_IDENT );
	assert( pNode->right->tag == K_COLUMN );
	this->TableName = pNode->left->data.val_str;
	Column auxcol;
	//auxcol.setName( string(pNode->left->data.val_str) + "." + string(pNode->right->data.val_str) );
	auxcol.setName( pNode->right->data.val_str );
	this->ColumnOnlyName = auxcol.getName();
	//debug13 - select will have changed the column names in the result table
	//to fully qualified names by the time Order::changeResult is called and
	//the changeResults of its columns are called. So until, identify columns by their
	//unqualified name because that's how AQ_Engine returns them.
	//but afterwards, identify them by their full name.
	//Also, check if it isn't possible to always use fully qualified names.
	//if( this->Context == K_ORDER || this->Context == K_GROUP )
	//{
		auxcol.setName( this->TableName + "." + this->ColumnOnlyName );
		this->ColumnName = auxcol.getName();
	//}
	//update: it IS possible

	if( this->Context != K_WHERE )
		return false; //must perform the query before we know the values

	return false; //debug13 temporary tryout because of a hunch: there is no
	//case in which obtaining a column in the WHERE clause is necessary
	//the only exceptions (I can think of now) would be:
	// - aggregate functions (but they are not allowed in WHERE)
	// - columns with only 1 value (they should be read and turned into scalars)
	//it is also damaging to obtain a column in the WHERE clause because
	//it will need to be read from the table if the above operation cannot be
	//solved now and it will not because Result will already be set.
	//All this results in a simple rule: don't load columns in changeQuery

	//TODO: only get a part at a time for a column

	/* Loop on Thesaurus Parts (biggest nLoopCnt is 999, three digit !) */
	int pErr;
	Column::Ptr column = new Column();
	column->setName( auxcol.getName() );
	for ( int nLoopCnt = 0; nLoopCnt < 1000; nLoopCnt++ )
		get_thesaurus_for_column_reference( *column, pNode, nLoopCnt, this->m_baseDesc, 
			this->m_settings->szThesaurusPath, &pErr );
	this->Result = column;
	return false;
}

//------------------------------------------------------------------------------
void ColumnVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft,
								VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	if( this->Result )
		return;
	for( size_t idx = 0; idx < table->Columns.size(); ++idx )
		if( table->Columns[idx]->getName() == this->ColumnName )
		{
			Column::Ptr column = table->Columns[idx];
			if( table->HasCount )
				column->setCount( table->Columns[table->Columns.size()-1] );
			this->Result = column;
			switch( this->Context )
			{
			case K_GROUP: column->GroupBy = true; break;
			case K_ORDER: column->OrderBy = true; break;
			}
			return;
		}
	assert( (table->Columns.size() == 0) ||
			(table->Columns.size() == 1) && table->HasCount );
}

//------------------------------------------------------------------------------
std::string ColumnVerb::getTableName() const{ return this->TableName; }
std::string ColumnVerb::getColumnName() const{ return this->ColumnName; };
std::string ColumnVerb::getColumnOnlyName() const{ return this->ColumnOnlyName; };

void ColumnVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( CommaVerb );

//------------------------------------------------------------------------------
CommaVerb::CommaVerb()
{}

//------------------------------------------------------------------------------
bool CommaVerb::changeQuery(	tnode* pStart, tnode* pNode,
								VerbResult::Ptr resLeft,
								VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	VerbResultArray::Ptr resArray = NULL;
	if( resLeft && resLeft->getType() == VerbResult::ARRAY )
	{
		resArray = static_pointer_cast<VerbResultArray>( resLeft );
		resArray->Results.push_back( resRight );
	}
	else
		if( resRight && resRight->getType() == VerbResult::ARRAY )
		{
			resArray = static_pointer_cast<VerbResultArray>( resRight );
			resArray->Results.push_front( resLeft );
		}
		else
		{
			resArray = new VerbResultArray();
			resArray->Results.push_back( resLeft );
			resArray->Results.push_back( resRight );
		}
	this->Result = resArray;
	return false;
}

//------------------------------------------------------------------------------
void CommaVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	VerbResultArray::Ptr resArray = NULL;
	if( resLeft && resLeft->getType() == VerbResult::ARRAY )
	{
		resArray = static_pointer_cast<VerbResultArray>( resLeft );
		resArray->Results.push_back( resRight );
	}
	else
		if( resRight && resRight->getType() == VerbResult::ARRAY )
		{
			resArray = static_pointer_cast<VerbResultArray>( resRight );
			resArray->Results.push_front( resLeft );
		}
		else
		{
			resArray = new VerbResultArray();
			resArray->Results.push_back( resLeft );
			resArray->Results.push_back( resRight );
		}
	//if( resRight ) //debug13 - LAG uses comma with K_VALUE which has no verb implemented yet
	this->Result = resArray;
}

void CommaVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( AndVerb );

//------------------------------------------------------------------------------
AndVerb::AndVerb()
{}

//------------------------------------------------------------------------------
void AndVerb::changeResult(	Table::Ptr table, 
							VerbResult::Ptr resLeft, 
							VerbResult::Ptr resRight, 
							VerbResult::Ptr resNext )
{
	if( this->Context != K_WHERE && this->Context != K_HAVING )
		return;
	if( !resLeft && !resRight )
		return;
	if( !resLeft && resRight )
	{
		this->Result = resRight;
		return;
	}
	if( resLeft && !resRight )
	{
		this->Result = resLeft;
		return;
	}

	//combine row validations
	if( resLeft->getType() != VerbResult::ROW_VALIDATION )
		throw verb_error(verb_error::VERB_BAD_SYNTAX, this->getVerbType());
	RowValidation::Ptr rv1 = dynamic_pointer_cast<RowValidation>( resLeft );

	if( resRight->getType() != VerbResult::ROW_VALIDATION )
		throw verb_error(verb_error::VERB_BAD_SYNTAX, this->getVerbType());
	RowValidation::Ptr rv2 = dynamic_pointer_cast<RowValidation>( resRight );

	if( rv1->ValidRows.size() != rv2->ValidRows.size() )
		throw verb_error(verb_error::VERB_BAD_SYNTAX, this->getVerbType());

	for( size_t idx2 = 0; idx2 < rv1->ValidRows.size(); ++idx2 )
		rv1->ValidRows[idx2] = rv1->ValidRows[idx2] && rv2->ValidRows[idx2];
	this->Result = rv1;
}

void AndVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( InVerb );

//------------------------------------------------------------------------------
InVerb::InVerb()
{}

//------------------------------------------------------------------------------
bool InVerb::preprocessQuery(	tnode* pStart, tnode* pNode, 
								tnode* pStartOriginal )
{
	if( !pNode || !pNode->left )
		return false;
	if( !(pNode->right->tag == K_IN_VALUES) )
		return false;
	return true;
}

//------------------------------------------------------------------------------
bool InVerb::changeQuery(	tnode* pStart, tnode* pNode,
							VerbResult::Ptr resLeft,
							VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	return true;
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( IntValueVerb );

//------------------------------------------------------------------------------
IntValueVerb::IntValueVerb()
{}

//------------------------------------------------------------------------------
bool IntValueVerb::preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal )
{
	assert( pNode->eNodeDataType == NODE_DATA_INT );
	this->Result = new Scalar(COL_TYPE_INT, ColumnItem((double)pNode->data.val_int));
	return false;
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( DoubleValueVerb );

//------------------------------------------------------------------------------
DoubleValueVerb::DoubleValueVerb()
{}

//------------------------------------------------------------------------------
bool DoubleValueVerb::preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal )
{
	assert( pNode->eNodeDataType == NODE_DATA_NUMBER );
	this->Result = new Scalar(COL_TYPE_DOUBLE, ColumnItem(pNode->data.val_number));
	return false;
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( StringValueVerb );

//------------------------------------------------------------------------------
StringValueVerb::StringValueVerb()
{}

//------------------------------------------------------------------------------
bool StringValueVerb::preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal )
{
	assert( pNode->eNodeDataType == NODE_DATA_STRING );
	this->Result = new Scalar(COL_TYPE_VARCHAR, ColumnItem(pNode->data.val_str));
	return false;
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( AsVerb );

//------------------------------------------------------------------------------
AsVerb::AsVerb()
{}

//------------------------------------------------------------------------------
void replaceTableIdent( tnode* pNode, const char* oldIdent, const char* newIdent )
{
	if( !pNode )
		return;
	if( pNode->tag == K_PERIOD && strcmp(oldIdent, pNode->left->data.val_str) == 0 )
		set_string_data( pNode->left, newIdent );

	replaceTableIdent( pNode->left, oldIdent, newIdent );
	replaceTableIdent( pNode->right, oldIdent, newIdent );
	replaceTableIdent( pNode->next, oldIdent, newIdent );
}

//------------------------------------------------------------------------------
bool AsVerb::preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal )
{
	switch( this->Context )
	{
	case K_FROM:
		assert( pNode && pNode->left );
		replaceTableIdent( pStart, pNode->right->data.val_str, pNode->left->data.val_str );
		delete_node( pNode->right );
		*pNode = *pNode->left; //no memory leaks
		return true;
	case K_SELECT:
		return false;
	default:
		throw verb_error(generic_error::INVALID_QUERY, this->getVerbType());
	}
	
	return false;
}

//------------------------------------------------------------------------------
void AsVerb::changeResult(	Table::Ptr table, 
							VerbResult::Ptr resLeft,
							VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	this->Result = resLeft;
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( AsteriskVerb );

//------------------------------------------------------------------------------
AsteriskVerb::AsteriskVerb()
{}

//------------------------------------------------------------------------------
bool AsteriskVerb::preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal )
{
	this->Result = new Asterisk();
	return false;
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( AscVerb );

//------------------------------------------------------------------------------
AscVerb::AscVerb()
{}

//------------------------------------------------------------------------------
void AscVerb::changeResult(	Table::Ptr table, 
							VerbResult::Ptr resLeft, 
							VerbResult::Ptr resRight, 
							VerbResult::Ptr resNext )
{
	//simply move the parameters up to the parent
	assert( resLeft && !resRight );
	this->Result = resLeft;
}