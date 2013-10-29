#include "ConversionVerbs.h"
#include "VerbVisitor.h"
#include <algorithm>
#include <aq/Exceptions.h>
#include <aq/DateConversion.h>

using namespace std;

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
bool CastVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	assert( pNode && pNode->right );
	switch( pNode->right->tag )
	{
	case K_INTEGER_TYPE: this->ConvertType = COL_TYPE_INT; break;
	case K_REAL_TYPE: this->ConvertType = COL_TYPE_DOUBLE; break;
	case K_STRING_TYPE: this->ConvertType = COL_TYPE_VARCHAR; break;
	default:
		assert( 0 );
	}
	return false;
}

//------------------------------------------------------------------------------
void Convert(	ColumnItem& dest, int destType, 
				const ColumnItem& source, int sourceType )
{
	char szBuffer[STR_BUF_SIZE]; // tma: FIXME
	memset(szBuffer, 0, STR_BUF_SIZE);
	switch( sourceType )
	{
	case COL_TYPE_INT:
	case COL_TYPE_BIG_INT:
		switch( destType )
		{
		case COL_TYPE_INT: 
		case COL_TYPE_BIG_INT: dest.numval = source.numval; return; break;
		case COL_TYPE_DOUBLE: dest.numval = source.numval; return;  break;
		case COL_TYPE_VARCHAR: 
			sprintf( szBuffer, "%lld", (llong) source.numval );
			dest.strval = szBuffer; 
			break;
		default:
			assert( 0 );
		}
		break;
	case COL_TYPE_DOUBLE: 
		switch( destType )
		{
		case COL_TYPE_INT: 
		case COL_TYPE_BIG_INT: dest.numval = (double) ((llong) source.numval); break;
		case COL_TYPE_DOUBLE: dest.numval = source.numval; return; break;
		case COL_TYPE_VARCHAR:
			doubleToString( szBuffer, source.numval );
			//sprintf( szBuffer, "%.2lf", source.numval );
			dest.strval = szBuffer; 
			break;
		default:
			assert( 0 );
		}
		break;
	case COL_TYPE_VARCHAR: 
		switch( destType )
		{
		case COL_TYPE_INT: 
		case COL_TYPE_BIG_INT:
			llong intval;
			if( StrToInt( source.strval.c_str(), &intval ) != 0 )
				throw verb_error(generic_error::VERB_TYPE_MISMATCH, -1); //debug13 - better handling here
			dest.numval = (double) intval;
			break;
		case COL_TYPE_DOUBLE:
			if( StrToDouble( source.strval.c_str(), &dest.numval ) != 0 )
				throw verb_error(generic_error::VERB_TYPE_MISMATCH, -1); //debug13 - better handling here
		case COL_TYPE_VARCHAR: dest.strval = source.strval; return; break;
		default:
			assert( 0 );
		}
		break;
	case COL_TYPE_DATE:
		switch( destType )
		{
		case COL_TYPE_VARCHAR:
			{
				DateConversion dateConverter;
				dest.strval = dateConverter.bigIntToDate((long long) source.numval);
			}
			break;
		default:
			assert( 0 );
		}
		break;
	default:
		assert( 0 );
	}
}

//------------------------------------------------------------------------------
void CastVerb::solve( VerbResult::Ptr resLeft )
{
	switch( resLeft->getType() )
	{
	case VerbResult::COLUMN:
		{
		  Column::Ptr column = boost::static_pointer_cast<Column>( resLeft );
			Column::Ptr newColumn = new Column( this->ConvertType );
			for( size_t idx = 0; idx < column->Items.size(); ++idx )
			{
				ColumnItem::Ptr newItem = new ColumnItem();
				Convert( *newItem, newColumn->Type, *column->Items[idx], column->Type );
				newColumn->Items.push_back( newItem );
			}
			newColumn->setCount( column->getCount() );
			this->Result = newColumn;
		}
		break;
	case VerbResult::SCALAR:
		{
		  Scalar::Ptr scalar = boost::static_pointer_cast<Scalar>( resLeft );
			Scalar::Ptr newScalar = new Scalar( this->ConvertType );
			Convert( newScalar->Item, newScalar->Type, scalar->Item, scalar->Type );
			this->Result = newScalar;
		}
		break;
	default:
		assert( 0 );
	}
}

//------------------------------------------------------------------------------
bool CastVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
							VerbResult::Ptr resLeft, VerbResult::Ptr resRight, 
							VerbResult::Ptr resNext )
{
	if( !resLeft )
		return false;
	assert( !resRight && !resNext );
	this->solve( resLeft );
	return false;
}

//------------------------------------------------------------------------------
void CastVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft,
								VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	if( this->Result )
		return;
	assert( resLeft && !resRight && !resNext );
	this->solve( resLeft );
}

//------------------------------------------------------------------------------
void CastVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
void NvlVerb::changeResult( Table::Ptr table, 
							VerbResult::Ptr resLeft, 
							VerbResult::Ptr resRight, 
							VerbResult::Ptr resNext )
{
	assert( resLeft && resRight );
	switch( resLeft->getType() )
	{
	case VerbResult::SCALAR:
		switch( resRight->getType() )
		{
		//debug13 - I don't have the possibility of NULL value in a scalar which 
		//means I have to rethink and redo my Scalar class (or perhaps give it up
		//and use a flag in Column instead of Scalar?)
		case VerbResult::SCALAR:
			this->Result = resLeft;
			break;
		case VerbResult::COLUMN:
			this->Result = resLeft;
			break;
		default:
			assert( 0 );
		}
		break;
	case VerbResult::COLUMN:
		{
		  Column::Ptr column = boost::static_pointer_cast<Column>( resLeft );
			Column::Ptr newColumn = new Column( "",
				static_cast<unsigned int>(column->ID), static_cast<unsigned int>(column->Size), column->Type );

			switch( resRight->getType() )
			{
			case VerbResult::SCALAR:
				{
				  Scalar::Ptr scalar = boost::static_pointer_cast<Scalar>( resRight );
					for( size_t idx = 0; idx < column->Items.size(); ++idx )
						if( !column->Items[idx] )
							newColumn->Items.push_back( new ColumnItem( scalar->Item ) );
						else
							newColumn->Items.push_back( column->Items[idx] );
				}
				break;
			case VerbResult::COLUMN:
				{
				  Column::Ptr column2 = boost::static_pointer_cast<Column>( resRight );
					for( size_t idx = 0; idx < column->Items.size(); ++idx )
						if( !column->Items[idx] )
							newColumn->Items.push_back( column2->Items[idx] );
						else
							newColumn->Items.push_back( column->Items[idx] );
				}
				break;
			default:
				assert( 0 );
			}
			this->Result = newColumn;
			break;
		}
	default:
		assert( 0 );
	}
}

//------------------------------------------------------------------------------
void NvlVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
bool DecodeVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	return false;
}

//------------------------------------------------------------------------------
void DecodeVerb::changeResult( Table::Ptr table, 
							   VerbResult::Ptr resLeft, 
							   VerbResult::Ptr resRight, 
							   VerbResult::Ptr resNext )
{
	assert( resLeft && resRight );
	Column::Ptr sourceCol;
	switch( resLeft->getType() )
	{
	case VerbResult::SCALAR:
		{
		  Scalar::Ptr scalar = boost::static_pointer_cast<Scalar>( resLeft );
			sourceCol = new Column();
			sourceCol->Type = scalar->Type;
			sourceCol->Items.push_back( new ColumnItem( scalar->Item ) );
			//debug13
			throw verb_error(generic_error::NOT_IMPLEMENTED, this->getVerbType());
		}
		break;
	case VerbResult::COLUMN:
	  sourceCol = boost::static_pointer_cast<Column>( resLeft );
		break;
	default:
		assert( 0 );
	}

	assert( resRight->getType() == VerbResult::ARRAY );
	VerbResultArray::Ptr searchList = boost::static_pointer_cast<VerbResultArray>( resRight );
	if( searchList->Results.size() == 0 )
		throw verb_error( generic_error::VERB_BAD_SYNTAX, this->getVerbType() );
	
	assert( searchList->Results.size() >= 1 );
	Scalar::Ptr defaultResult = NULL;
	size_t nrResults = searchList->Results.size();
	if( nrResults == 1 ||
		nrResults > 2 &&
		searchList->Results[nrResults-1]->getType() == VerbResult::SCALAR )
	{
	  defaultResult = boost::static_pointer_cast<Scalar>( searchList->Results[nrResults-1] );
		searchList->Results.pop_back();
	}

	vector<Scalar::Ptr> search;
	vector<Scalar::Ptr> result;
	if( searchList->Results.size() > 1 )
	{
		assert( searchList->Results[0]->getType() == VerbResult::SCALAR );
		assert( searchList->Results[1]->getType() == VerbResult::SCALAR );
		search.push_back( boost::static_pointer_cast<Scalar>( searchList->Results[0] ) );
		result.push_back( boost::static_pointer_cast<Scalar>( searchList->Results[1] ) );
		for( size_t idx = 2; idx < searchList->Results.size(); ++idx )
		{
			assert( searchList->Results[idx]->getType() == VerbResult::ARRAY );
			VerbResultArray::Ptr searchResultPair = boost::static_pointer_cast<VerbResultArray>( searchList->Results[idx] );
			assert( searchResultPair->Results[0]->getType() == VerbResult::SCALAR );
			assert( searchResultPair->Results[1]->getType() == VerbResult::SCALAR );
			search.push_back( boost::static_pointer_cast<Scalar>( searchResultPair->Results[0] ) );
			result.push_back( boost::static_pointer_cast<Scalar>( searchResultPair->Results[1] ) );
		}
	}
	
	Column::Ptr destCol = new Column();
	destCol->setCount( sourceCol->getCount() );
	if( result.size() > 0 )
		destCol->Type = result[0]->Type;
	else if( defaultResult )
		destCol->Type = defaultResult->Type;
	else
		assert( 0 );
	destCol->setCount( sourceCol->getCount() );

	for( size_t idx = 0; idx < sourceCol->Items.size(); ++idx )
	{
		bool found = false;
		for( size_t idx2 = 0; idx2 < search.size(); ++idx2 )
			if( ColumnItem::equal(	sourceCol->Items[idx].get(), 
						&search[idx2]->Item, 
						sourceCol->Type ) )
			{
				destCol->Items.push_back( new ColumnItem( result[idx2]->Item ) );
				found = true;
				break;
			}
		if( !found )
			if( defaultResult )
				destCol->Items.push_back( new ColumnItem( defaultResult->Item ) );
			else
				destCol->Items.push_back( NULL );
	}

	this->Result = destCol;
}

//------------------------------------------------------------------------------
void DecodeVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

}
}
