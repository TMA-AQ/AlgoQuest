#include "ComparisonVerbs.h"
#include "ExprTransform.h"
#include <aq/Exceptions.h>
#include <aq/Logger.h>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace aq;
using namespace std;
using namespace boost;

//------------------------------------------------------------------------------
VERB_IMPLEMENT( ComparisonVerb );

//------------------------------------------------------------------------------
ComparisonVerb::ComparisonVerb()
{}

//------------------------------------------------------------------------------
bool ComparisonVerb::changeQuery(	tnode* pStart, tnode* pNode,
									VerbResult::Ptr resLeft, 
									VerbResult::Ptr resRight, 
									VerbResult::Ptr resNext )
{
	assert( pNode );
	assert( pNode->eNodeDataType != NODE_DATA_STRING );
	int pErr = 0;
	//the argument given to expression_transform will be destroyed if the function
	//is successful
	tnode* pNodeClone = new_node( pNode->tag );
	*pNodeClone = *pNode;
	tnode* newNode = pNodeClone;
	if( this->Context == K_WHERE )
	{
		boost::posix_time::ptime begin(boost::posix_time::microsec_clock::local_time());
		newNode = expression_transform( pNodeClone, this->m_baseDesc, this->m_settings->szThesaurusPath, &pErr );
		boost::posix_time::ptime end(boost::posix_time::microsec_clock::local_time());
		std::ostringstream oss;
		oss << "expression_transform elapsed time: " << (end - begin) << " ms" << std::endl;
		aq::Logger::getInstance().log(AQ_INFO, "%s\n", oss.str().c_str());
	}
	
	if( newNode == pNodeClone ) //expression not transformed
	{
		delete_node( pNodeClone );
		if( !pNode->left || !pNode->right )
			throw verb_error(generic_error::GENERIC, this->getVerbType());
		if( this->Context == K_WHERE &&
			(pNode->left->tag == K_PERIOD &&
			pNode->right->tag == K_PERIOD ||
			pNode->left->tag == K_PERIOD &&
			resRight && resRight->getType() == VerbResult::SCALAR ||
			pNode->right->tag == K_PERIOD &&
			resLeft && resLeft->getType() == VerbResult::SCALAR ||
			(pNode->left->tag == K_INNER || pNode->left->tag == K_OUTER) &&
			(pNode->right->tag == K_INNER || pNode->right->tag == K_OUTER)) )
			return true; //comparison can be handled by the engine
		else
		{
			pNode->tag = K_DELETED;
			return false; //comparison cannot be handled by the engine
		}
	}
	if( !newNode || pErr != 0 ) //error
	{
		if( newNode )
			delete_node( newNode );
		delete_node( pNodeClone );
		throw verb_error(generic_error::GENERIC, this->getVerbType());
	}
	//pNodeClone already deleted by expression_transform
	assert( newNode->eNodeDataType != NODE_DATA_STRING );
	*pNode = *newNode;
	delete_node( newNode );
	return true;
}

//------------------------------------------------------------------------------
void ComparisonVerb::changeResult(	Table::Ptr table, 
									VerbResult::Ptr resLeft, 
									VerbResult::Ptr resRight, 
									VerbResult::Ptr resNext )
{
	if( !resLeft || !resRight )
		throw verb_error(generic_error::VERB_BAD_SYNTAX, this->getVerbType());
	//we have at least 1 column
	Column::Ptr column;
	VerbResult::Ptr other;
	if( resLeft->getType() == VerbResult::SCALAR )
	{
		if( resRight->getType() != VerbResult::COLUMN )
			throw verb_error(generic_error::VERB_BAD_SYNTAX, this->getVerbType());
		column = dynamic_pointer_cast<Column>( resRight );
		other = resLeft;
	}
	else
	{
		if( resLeft->getType() != VerbResult::COLUMN )
			throw verb_error(generic_error::VERB_BAD_SYNTAX, this->getVerbType());
		column = dynamic_pointer_cast<Column>( resLeft );
		other = resRight;
	}
	RowValidation::Ptr rowValidation = new RowValidation();
	vector<bool>& validRows = rowValidation->ValidRows;
	validRows.resize( column->Items.size(), false );
	if( other->getType() == VerbResult::SCALAR )
	{
		Scalar::Ptr scalar = dynamic_pointer_cast<Scalar>( other );
		for( size_t idx = 0; idx < column->Items.size(); ++idx )
			validRows[idx] = this->compare(	column->Items[idx].get(), 
											&scalar->Item, 
											column->Type );
	}
	else
	{
		Column::Ptr column2 = dynamic_pointer_cast<Column>( other );
		if( column->Items.size() != column2->Items.size() )
			throw verb_error(generic_error::VERB_BAD_SYNTAX, this->getVerbType());
		
		for( size_t idx = 0; idx < column->Items.size(); ++idx )
			validRows[idx] = this->compare(	column->Items[idx].get(), 
											column2->Items[idx].get(), 
											column->Type );
	}
	this->Result = rowValidation;
}

//------------------------------------------------------------------------------
bool ComparisonVerb::compare(	ColumnItem* item1, 
								ColumnItem* item2, 
								ColumnType type )
{
	throw verb_error(generic_error::VERB_BAD_SYNTAX, this->getVerbType());
	return false;
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( EqVerb );

//------------------------------------------------------------------------------
EqVerb::EqVerb()
{}

//------------------------------------------------------------------------------
bool EqVerb::compare(	ColumnItem* item1, 
						ColumnItem* item2, 
						ColumnType type )
{
	return equal( item1, item2, type );
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( JeqVerb );

//------------------------------------------------------------------------------
JeqVerb::JeqVerb()
{}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( JautoVerb );

//------------------------------------------------------------------------------
JautoVerb::JautoVerb()
{}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( LtVerb );

//------------------------------------------------------------------------------
LtVerb::LtVerb()
{}

//------------------------------------------------------------------------------
bool LtVerb::compare(	ColumnItem* item1, 
						ColumnItem* item2, 
						ColumnType type )
{
	return lessThan( item1, item2, type );
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( LeqVerb );

//------------------------------------------------------------------------------
LeqVerb::LeqVerb()
{}

//------------------------------------------------------------------------------
bool LeqVerb::compare(	ColumnItem* item1, 
						ColumnItem* item2, 
						ColumnType type )
{
	return equal(item1, item2, type) || lessThan( item1, item2, type );
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( GtVerb );

//------------------------------------------------------------------------------
GtVerb::GtVerb()
{}

//------------------------------------------------------------------------------
bool GtVerb::compare(	ColumnItem* item1, 
						ColumnItem* item2, 
						ColumnType type )
{
	return !equal(item1, item2, type) && !lessThan( item1, item2, type );
}


//------------------------------------------------------------------------------
VERB_IMPLEMENT( GeqVerb );

//------------------------------------------------------------------------------
GeqVerb::GeqVerb()
{}

//------------------------------------------------------------------------------
bool GeqVerb::compare(	ColumnItem* item1, 
						ColumnItem* item2, 
						ColumnType type )
{
	return !lessThan( item1, item2, type );
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( BetweenVerb );

//------------------------------------------------------------------------------
BetweenVerb::BetweenVerb()
{}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( NotBetweenVerb );

//------------------------------------------------------------------------------
NotBetweenVerb::NotBetweenVerb()
{}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( LikeVerb );

//------------------------------------------------------------------------------
LikeVerb::LikeVerb()
{}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( NotLikeVerb );

//------------------------------------------------------------------------------
NotLikeVerb::NotLikeVerb()
{}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( JinfVerb );

//------------------------------------------------------------------------------
JinfVerb::JinfVerb()
{}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( JieqVerb );

//------------------------------------------------------------------------------
JieqVerb::JieqVerb()
{}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( JsupVerb );

//------------------------------------------------------------------------------
JsupVerb::JsupVerb()
{}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( JseqVerb );

//------------------------------------------------------------------------------
JseqVerb::JseqVerb()
{}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( NeqVerb );

//------------------------------------------------------------------------------
NeqVerb::NeqVerb()
{}

//------------------------------------------------------------------------------
bool NeqVerb::compare(	ColumnItem* item1, 
						ColumnItem* item2, 
						ColumnType type )
{
	return !equal( item1, item2, type );
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( JneqVerb );

//------------------------------------------------------------------------------
JneqVerb::JneqVerb()
{}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( IsVerb );

//------------------------------------------------------------------------------
IsVerb::IsVerb(): IsNot(false)
{}

//------------------------------------------------------------------------------
bool IsVerb::preprocessQuery(	tnode* pStart, tnode* pNode, 
								tnode* pStartOriginal )
{

	if( !pNode->right ||
		!(pNode->right->tag == K_NOT && pNode->right->left &&
		pNode->right->left->tag == K_NULL ||
		pNode->right->tag == K_NULL) )
		throw generic_error(generic_error::NOT_IMPLEMENED, "");
	if( pNode->right->tag == K_NOT && pNode->right->left->tag == K_NULL )
		this->IsNot = true;
	return false;
}

//------------------------------------------------------------------------------
void IsVerb::changeResult(	Table::Ptr table, 
							VerbResult::Ptr resLeft, 
							VerbResult::Ptr resRight, 
							VerbResult::Ptr resNext )
{
	if( !resLeft )
		throw verb_error(generic_error::VERB_BAD_SYNTAX, this->getVerbType());
	Column::Ptr column;
	if( resLeft->getType() != VerbResult::COLUMN )
		throw verb_error(generic_error::VERB_BAD_SYNTAX, this->getVerbType());
	column = dynamic_pointer_cast<Column>( resLeft );
	RowValidation::Ptr rowValidation = new RowValidation();
	vector<bool>& validRows = rowValidation->ValidRows;
	validRows.resize( column->Items.size(), false );
	for( size_t idx = 0; idx < column->Items.size(); ++idx )
		validRows[idx] = (column->Items[idx] == NULL) == !this->IsNot;
	this->Result = rowValidation;
}