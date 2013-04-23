#pragma once

#include "VerbNode.h"
#include <aq/RowProcess_Intf.h>
#include <list>

//------------------------------------------------------------------------------
class SelectVerb: public VerbNode
{
	VERB_DECLARE( SelectVerb );
public:
	virtual int getVerbType() const { return K_SELECT; };
	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );

	virtual void setBaseDesc(Base * baseDesc) 
	{ 
		m_baseDesc = baseDesc;
	}
	
	virtual void accept(VerbVisitor* visitor);

private:
	std::vector<std::string> Columns;
	std::vector<std::string> ColumnsDisplay;
	Base * m_baseDesc;
};

//------------------------------------------------------------------------------
class WhereVerb: public VerbNode
{
	VERB_DECLARE( WhereVerb );
public:
	virtual int getVerbType() const { return K_WHERE; };
	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
class OrderVerb: public VerbNode
{
	VERB_DECLARE( OrderVerb );
public:
	virtual int getVerbType() const { return K_ORDER; };
	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};

//------------------------------------------------------------------------------
class ByVerb: public VerbNode
{
	VERB_DECLARE( ByVerb );
public:
	virtual int getVerbType() const { return K_BY; };
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
class FromVerb: public VerbNode
{
	VERB_DECLARE( FromVerb );
public:
	virtual int getVerbType() const { return K_FROM; };
	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void setBaseDesc(Base * baseDesc) 
  { 
    m_baseDesc = baseDesc;
  }
	virtual void accept(VerbVisitor* visitor);
  const std::list<std::string>& getTables() const { return this->tables; };
private:
  Base * m_baseDesc;
  std::list<std::string> tables;
};

//------------------------------------------------------------------------------
class GroupVerb: public VerbNode
{
	VERB_DECLARE( GroupVerb );
public:
	virtual int getVerbType() const { return K_GROUP; };
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void addResult( aq::RowProcess_Intf::row_t& row, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  void accept(VerbVisitor* visitor);
private:
  aq::RowProcess_Intf::row_t row_acc;
  // std::list<RowProcess_Intf::row_t> rows;
};

//------------------------------------------------------------------------------
class HavingVerb: public VerbNode
{
	VERB_DECLARE( HavingVerb );
public:
	virtual int getVerbType() const { return K_HAVING; };
	virtual bool preprocessQuery(	tnode* pStart, tnode* pNode, 
									tnode* pStartOriginal );
	virtual void changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext );
};
