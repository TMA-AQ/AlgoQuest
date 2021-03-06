#pragma once

#include "VerbNode.h"
#include <aq/RowProcess_Intf.h>
#include <list>

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
class SelectVerb: public VerbNode
{
public:
	virtual int getVerbType() const { return K_SELECT; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual bool changeQuery( aq::tnode* pStart, aq::tnode* pNode,
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
public:
	virtual int getVerbType() const { return K_WHERE; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual bool changeQuery( aq::tnode* pStart, aq::tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
class OrderVerb: public VerbNode
{
public:
	virtual int getVerbType() const { return K_ORDER; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual bool changeQuery( aq::tnode* pStart, aq::tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
class ByVerb: public VerbNode
{
public:
	virtual int getVerbType() const { return K_BY; };
	virtual bool changeQuery( aq::tnode* pStart, aq::tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
class FromVerb: public VerbNode
{
public:
	virtual int getVerbType() const { return K_FROM; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual bool changeQuery( aq::tnode* pStart, aq::tnode* pNode,
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
public:
	virtual int getVerbType() const { return K_GROUP; };
  virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual bool changeQuery( aq::tnode* pStart, aq::tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void addResult(aq::Row& row);
  void accept(VerbVisitor* visitor);
  virtual void setSettings(Settings* settings)
  {
    this->useRowResolver = true;
  }
private:
  aq::Row row_prv;
  aq::Row row_acc;
  bool useRowResolver;
  // std::list<RowProcess_Intf::row_t> rows;
};

//------------------------------------------------------------------------------
class HavingVerb: public VerbNode
{
public:
	virtual int getVerbType() const { return K_HAVING; };
	virtual bool preprocessQuery(	aq::tnode* pStart, aq::tnode* pNode, 
									aq::tnode* pStartOriginal );
	virtual void changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor* visitor);
};

}
}
