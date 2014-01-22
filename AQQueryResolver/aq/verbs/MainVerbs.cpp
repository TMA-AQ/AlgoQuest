#include "MainVerbs.h"
#include "VerbVisitor.h"
#include <algorithm>
#include <aq/parser/ID2Str.h>
#include <aq/Exceptions.h>
#include <aq/TreeUtilities.h>
#include <aq/Column2Table.h>
#include <aq/Logger.h>

using namespace std;

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
bool SelectVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	assert( pStartOriginal->tag == pNode->tag );
	vector<aq::tnode*> columns;
	columns.clear();
	aq::util::getColumnsList( pStartOriginal->left, columns );
	for( size_t idx = 0; idx < columns.size(); ++idx )
	{
		std::string name;
		aq::util::extractName( columns[idx], name );
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
		aq::util::solveSelectStar( pNode, this->m_baseDesc, this->Columns, this->ColumnsDisplay );
		return false;
	}
	vector<aq::tnode*> columns;
	aq::util::getColumnsList( pNode->left, columns );
	for( size_t idx = 0; idx < columns.size(); ++idx )
	{
		std::string name;
		if( columns[idx]->tag == K_PERIOD )
		{
			name += columns[idx]->left->getData().val_str;
			name += ".";
			name += columns[idx]->right->getData().val_str;
		}
		else
		{
			if( columns[idx]->tag == K_AS && columns[idx]->left->tag == K_PERIOD )
			{
				name += columns[idx]->left->left->getData().val_str;
				name += ".";
				name += columns[idx]->left->right->getData().val_str;
			}
			columns[idx]->tag = K_DELETED;
			columns[idx] = nullptr;
		}
		this->Columns.push_back( name );
	}

	assert( this->Columns.size() == this->ColumnsDisplay.size() );
	aq::util::getAllColumns( pNode, columns );

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
	pNode->left = new aq::tnode( K_COMMA );
	pNode = pNode->left;
	pNode->right = pAuxNode;
	
	for( size_t idx = this->Columns.size(); idx < columns.size() - 1; ++idx )
	{
		pNode->left = new aq::tnode( K_COMMA );
		pNode = pNode->left;
		pNode->right = columns[idx];
	}
	pNode->left = columns[columns.size() - 1];

	return false;
}

//------------------------------------------------------------------------------
void SelectVerb::changeResult(Table::Ptr table, 
                              VerbResult::Ptr resLeft,
                              VerbResult::Ptr resRight, 
                              VerbResult::Ptr resNext )
{
  assert(false);
}

void SelectVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool WhereVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	// eliminate K_NOT
	aq::util::processNot(pNode->left, false);
	return false;
}

//------------------------------------------------------------------------------
bool WhereVerb::changeQuery( aq::tnode* pStart, aq::tnode* pNode,
	VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	aq::util::addInnerOuterNodes( pNode->left, K_INNER, K_INNER );
	return false;
}

//------------------------------------------------------------------------------
void WhereVerb::changeResult( Table::Ptr table, 
	VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	assert(false);
}

void WhereVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool OrderVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	vector<aq::tnode*> columns;
	aq::util::getColumnsList( pNode->left->left, columns );
	vector<aq::tnode*> selectColumns;
	aq::util::getColumnsList( pStart->left, selectColumns );
	for( size_t idx = 0; idx < columns.size(); ++idx )
  {
		if( columns[idx]->tag == K_IDENT || 
			columns[idx]->tag == K_COLUMN )
		{
			int colIdx = -1;
			for( size_t idx2 = 0; idx2 < selectColumns.size(); ++idx2 )
      {
				if( selectColumns[idx2] &&
					selectColumns[idx2]->tag == K_AS &&
					strcmp( selectColumns[idx2]->right->getData().val_str,
						columns[idx]->getData().val_str ) == 0
					)
				{
					colIdx = (int) idx2;
					break;
				}
      }
			if( colIdx < 0 )
      {
				throw verb_error(generic_error::INVALID_QUERY, this->getVerbType());
      }
			columns[idx]->tag = K_INTEGER;
			columns[idx]->set_int_data( colIdx + 1 );
		}
  }
	return false;
}

//------------------------------------------------------------------------------
bool OrderVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
								VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	// pNode->tag = K_DELETED;
	return false;
}

//------------------------------------------------------------------------------
void OrderVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	assert(false);
}

//------------------------------------------------------------------------------
void OrderVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

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
	assert(false);
}

//------------------------------------------------------------------------------
void ByVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool FromVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
  if (enforce_qualified_column_reference(pStart, *this->m_baseDesc) != 0)
  {
		throw generic_error(generic_error::INVALID_QUERY, "Error : No or bad tables specified in SQL SELECT ... FROM Statement");
  }
  aq::util::getTablesList(pNode, this->tables);
	return false;
}

//------------------------------------------------------------------------------
bool FromVerb::changeQuery( aq::tnode* pStart, aq::tnode* pNode,
	VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	aq::util::solveOneTableInFrom( pStart, this->m_baseDesc );
	aq::util::moveFromJoinToWhere( pStart, this->m_baseDesc );
	return false;
}

//------------------------------------------------------------------------------
void FromVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool GroupVerb::preprocessQuery(aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal)
{
  return this->useRowResolver;
}

//------------------------------------------------------------------------------
bool GroupVerb::changeQuery(aq::tnode* pStart, aq::tnode* pNode,
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
  assert(false);
}

bool is_new_group(const aq::row_item_t::item_t& i1, const aq::row_item_t::item_t& i2, aq::ColumnType type)
{
  bool new_group = false;
  switch (type)
  {
  case aq::ColumnType::COL_TYPE_INT:
    {
      const aq::ColumnItem<int32_t>& i1_tmp = boost::get<aq::ColumnItem<int32_t> >(i1);
      const aq::ColumnItem<int32_t>& i2_tmp = boost::get<aq::ColumnItem<int32_t> >(i2);
      new_group = !ColumnItem<int32_t>::equal(i1_tmp, i2_tmp);
    }
    break;
  case aq::ColumnType::COL_TYPE_BIG_INT:
  case aq::ColumnType::COL_TYPE_DATE:
    {
      const aq::ColumnItem<int64_t>& i1_tmp = boost::get<aq::ColumnItem<int64_t> >(i1);
      const aq::ColumnItem<int64_t>& i2_tmp = boost::get<aq::ColumnItem<int64_t> >(i2);
      new_group = !ColumnItem<int64_t>::equal(i1_tmp, i2_tmp);
    }
    break;
  case aq::ColumnType::COL_TYPE_DOUBLE:
    {
      const aq::ColumnItem<double>& i1_tmp = boost::get<aq::ColumnItem<double> >(i1);
      const aq::ColumnItem<double>& i2_tmp = boost::get<aq::ColumnItem<double> >(i2);
      new_group = !ColumnItem<double>::equal(i1_tmp, i2_tmp);
    }
    break;
  case aq::ColumnType::COL_TYPE_VARCHAR:
    {
      const aq::ColumnItem<char*>& i1_tmp = boost::get<aq::ColumnItem<char*> >(i1);
      const aq::ColumnItem<char*>& i2_tmp = boost::get<aq::ColumnItem<char*> >(i2);
      new_group = !ColumnItem<char*>::equal(i1_tmp, i2_tmp);
    }
    break;
  }
  return new_group;
}

//------------------------------------------------------------------------------
void GroupVerb::addResult(aq::Row& row)
{
  assert((this->row_acc.computedRow.size() == 0) || (row.flush) || (row.computedRow.size() == this->row_acc.computedRow.size()));
  assert((this->row_prv.initialRow.size() == 0) || (row.flush) || (row.initialRow.size() == this->row_prv.initialRow.size()));
  assert(this->row_prv.computedRow.size() == this->row_acc.computedRow.size());
  
  row.completed = false;

  // flush
  if (row.flush)
  {
    row.completed = true;
    row.computedRow.clear();
    std::copy(this->row_acc.computedRow.begin(), this->row_acc.computedRow.end(), std::back_inserter<aq::Row::row_t>(row.computedRow));
    return;
  }

  // check if new group
  bool new_group = false;
  if (row_prv.computedRow.size() > 0)
  {
    for (size_t i = 0; i < row.initialRow.size(); ++i)
    {
      // row_prv.initialRow[i].type
      if (row.initialRow[i].grouped)
      {
        new_group = is_new_group(row_prv.initialRow[i].item, row.initialRow[i].item, row.initialRow[i].type);
      }
    }
  }
  row.completed = new_group;

  // store previous
  row_prv = row;

  //
  if (new_group)
  {
    row.computedRow.clear();
    std::copy(this->row_acc.computedRow.begin(), this->row_acc.computedRow.end(), std::back_inserter<aq::Row::row_t>(row.computedRow));
    this->row_acc.computedRow.clear();
  }
   
  // compute and store in row_acc
  if (row_acc.computedRow.empty())
  {
    std::copy(row_prv.computedRow.begin(), row_prv.computedRow.end(), std::back_inserter<aq::Row::row_t>(row_acc.computedRow));
    row_acc.count= row_prv.count;
    for (size_t i = 0; i < row.computedRow.size(); ++i)
    {
      switch (row.computedRow[i].aggFunc)
      {
      case SUM:

        // FIXME : TODO
        // row_acc.computedRow[i].item->numval *= row_acc.count;
        
        break;
      default:

        throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "aggregate function not implemented");

        break;
      }
    }
  }
  else
  {
    for (size_t i = 0; i < row.computedRow.size(); ++i)
    {
      switch (row.computedRow[i].type)
      {
      case aq::ColumnType::COL_TYPE_INT:
        {
          auto& i1 = boost::get<aq::ColumnItem<int32_t> >(row_acc.computedRow[i].item);
          auto& i2 = boost::get<aq::ColumnItem<int32_t> >(row_acc.computedRow[i].item);
          i1.applyAggregate(row.computedRow[i].aggFunc, i2);
        }
        break;
      case aq::ColumnType::COL_TYPE_BIG_INT:
      case aq::ColumnType::COL_TYPE_DATE:
        {
          auto& i1 = boost::get<aq::ColumnItem<int64_t> >(row_acc.computedRow[i].item);
          auto& i2 = boost::get<aq::ColumnItem<int64_t> >(row_acc.computedRow[i].item);
          i1.applyAggregate(row.computedRow[i].aggFunc, i2);
        }
        break;
      case aq::ColumnType::COL_TYPE_DOUBLE:
        {
          auto& i1 = boost::get<aq::ColumnItem<double> >(row_acc.computedRow[i].item);
          auto& i2 = boost::get<aq::ColumnItem<double> >(row_acc.computedRow[i].item);
          i1.applyAggregate(row.computedRow[i].aggFunc, i2);
        }
        break;
      case aq::ColumnType::COL_TYPE_VARCHAR:
        {
          auto& i1 = boost::get<aq::ColumnItem<char*> >(row_acc.computedRow[i].item);
          auto& i2 = boost::get<aq::ColumnItem<char*> >(row_acc.computedRow[i].item);
          i1.applyAggregate(row.computedRow[i].aggFunc, i2);
        }
        break;
      }
    }
    this->row_acc.count += this->row_prv.count;
  }

}

//------------------------------------------------------------------------------
void GroupVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool HavingVerb::preprocessQuery(	aq::tnode* pStart, aq::tnode* pNode, 
									aq::tnode* pStartOriginal )
{
	//eliminate K_NOT
	aq::util::processNot( pNode->left, false );
	return false;
}

//------------------------------------------------------------------------------
void HavingVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
  assert(false);
}

//------------------------------------------------------------------------------
void HavingVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

}
}
