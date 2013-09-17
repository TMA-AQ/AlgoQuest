#include "CaseVerb.h"
#include "VerbVisitor.h"
#include <aq/Exceptions.h>
#include <algorithm>

using namespace std;

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
VERB_IMPLEMENT(CaseVerb);

//------------------------------------------------------------------------------
CaseVerb::CaseVerb()
{}

//------------------------------------------------------------------------------
void CaseVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
	assert( resLeft->getType() == VerbResult::COLUMN );
	Column::Ptr testColumn = static_pointer_cast<Column>(resLeft);
	assert( resRight->getType() == VerbResult::ARRAY );
	VerbResultArray::Ptr testArray = static_pointer_cast<VerbResultArray>(resRight);
	assert( testArray->Results.size() == 2 );
	assert( testArray->Results[0]->getType() == VerbResult::ARRAY );
	assert( testArray->Results[1]->getType() == VerbResult::ARRAY );

	deque<VerbResult::Ptr>& conditions = static_pointer_cast<VerbResultArray>(testArray->Results[0])->Results;
	deque<VerbResult::Ptr>& results = static_pointer_cast<VerbResultArray>(testArray->Results[1])->Results;
	assert( conditions.size() == results.size() );
	assert( conditions.size() > 0 );
	//remember that the when list is retrieved from end to start
	bool hasElse = !conditions[0];

	ColumnType resultType;
	if( results[0]->getType() == VerbResult::COLUMN )
		resultType = ((Column*) results[0].get())->Type;
	else if( results[0]->getType() == VerbResult::SCALAR )
		resultType = ((Scalar*) results[0].get())->Type;
	else
		assert( 0 );
	
	Column::Ptr resultColumn = new Column(resultType);
	for( size_t idx = 0; idx < testColumn->Items.size(); ++idx )
	{
		for( size_t idx2 = hasElse ? 1 : 0; idx2 < conditions.size(); ++idx2 )
		{
			ColumnItem* condItem = NULL;
			ColumnType condType;
			if( conditions[idx2]->getType() == VerbResult::COLUMN )
			{
				condType = ((Column*) conditions[idx2].get())->Type;
				condItem = ((Column*) conditions[idx2].get())->Items[idx].get();
			}
			else
			{
				condType = ((Scalar*) conditions[idx2].get())->Type;
				condItem = &((Scalar*) conditions[idx2].get())->Item;
			}

			if( condType != testColumn->Type )
				throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType());

			if( ColumnItem::equal(	testColumn->Items[idx].get(), condItem, condType) )
			{
				ColumnItem::Ptr resItem = NULL;
				ColumnType resType;
				if( results[idx2]->getType() == VerbResult::COLUMN )
				{
					resType = ((Column*) results[idx2].get())->Type;
					resItem = ((Column*) results[idx2].get())->Items[idx].get();
				}
				else
				{
					resType = ((Scalar*) results[idx2].get())->Type;
					resItem = new ColumnItem(((Scalar*) results[idx2].get())->Item);
				}
				if( resType != resultType )
					throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType());

				resultColumn->Items.push_back( resItem );
				break;
			}
		}
		if( resultColumn->Items.size() != (idx + 1) )
		{
			if( hasElse )
			{
				ColumnItem::Ptr resItem = NULL;
				if( results[0]->getType() == VerbResult::COLUMN )
					resItem = ((Column*) results[0].get())->Items[idx];
				else
					resItem = new ColumnItem(((Scalar*) results[0].get())->Item);
				resultColumn->Items.push_back( resItem );
			}
			else
				resultColumn->Items.push_back( NULL ); //no element was found
		}
	}
	this->Result = resultColumn;
}

//------------------------------------------------------------------------------
void CaseVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT(WhenVerb);

//------------------------------------------------------------------------------
WhenVerb::WhenVerb()
{}

//------------------------------------------------------------------------------
void WhenVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
	VerbResultArray::Ptr result = new VerbResultArray();
	VerbResultArray::Ptr resArray = NULL;
	if( resNext )
	{
		assert( resNext->getType() == VerbResult::ARRAY );
		resArray = static_pointer_cast<VerbResultArray>(resNext);
		
	}
	else
	{
		resArray = new VerbResultArray();
		resArray->Results.push_back( new VerbResultArray() );
		resArray->Results.push_back( new VerbResultArray() );
	}
	assert( resArray->Results.size() == 2 );
	assert( resArray->Results[0]->getType() == VerbResult::ARRAY );
	static_pointer_cast<VerbResultArray>(resArray->Results[0])->Results.push_back( resLeft );
	assert( resArray->Results[1]->getType() == VerbResult::ARRAY );
	static_pointer_cast<VerbResultArray>(resArray->Results[1])->Results.push_back( resRight );
	this->Result = resArray;
}

//------------------------------------------------------------------------------
void WhenVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT(ElseVerb);

//------------------------------------------------------------------------------
ElseVerb::ElseVerb()
{}

//------------------------------------------------------------------------------
void ElseVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
	
	assert( !resNext );
	assert( !resRight );
	VerbResultArray::Ptr resArray = new VerbResultArray();
	VerbResultArray::Ptr conditions = new VerbResultArray();
	VerbResultArray::Ptr results = new VerbResultArray();
	resArray->Results.push_back( conditions );
	resArray->Results.push_back( results );
	conditions->Results.push_back( NULL );
	results->Results.push_back( resLeft );
	this->Result = resArray;
}

//------------------------------------------------------------------------------
void ElseVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

}
}
