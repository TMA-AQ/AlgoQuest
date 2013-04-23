#pragma once

#include "Verb.h"
#include <list>

//------------------------------------------------------------------------------
class SelectVerb: public Verb
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
class WhereVerb: public Verb
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
class OrderVerb: public Verb
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
class ByVerb: public Verb
{
	VERB_DECLARE( ByVerb );
public:
	virtual int getVerbType() const { return K_BY; };
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};

//------------------------------------------------------------------------------
class FromVerb: public Verb
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
private:
  Base * m_baseDesc;
  std::list<std::string> tables;
};

//------------------------------------------------------------------------------
class GroupVerb: public Verb
{
	VERB_DECLARE( GroupVerb );
public:
	virtual int getVerbType() const { return K_GROUP; };
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};

//------------------------------------------------------------------------------
class HavingVerb: public Verb
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
