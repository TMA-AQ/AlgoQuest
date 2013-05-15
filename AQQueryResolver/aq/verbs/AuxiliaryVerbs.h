#pragma once

#include "VerbNode.h"

//------------------------------------------------------------------------------
class ColumnVerb: public VerbNode
{
	VERB_DECLARE( ColumnVerb );
public:
	virtual int getVerbType() const { return K_PERIOD; };
	virtual bool changeQuery( aq::tnode* pStart, aq::tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void addResult(	aq::RowProcess_Intf::Row& row, 
    VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );

	std::string getTableName() const;
	std::string getColumnName() const;
	std::string getColumnOnlyName() const;
  
  virtual void setBaseDesc(Base * baseDesc) 
  { 
    m_baseDesc = baseDesc;
  }
  
  virtual void setSettings(TProjectSettings * settings) 
  {
    m_settings = settings;
  }

	virtual void accept(VerbVisitor*);

private:
	std::string TableName, ColumnName, ColumnOnlyName;
  Base * m_baseDesc;
  TProjectSettings * m_settings;
};

//------------------------------------------------------------------------------
class CommaVerb: public VerbNode
{
	VERB_DECLARE( CommaVerb );
public:
	virtual int getVerbType() const { return K_COMMA; };
	bool changeQuery( aq::tnode* pStart, aq::tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void addResult( aq::RowProcess_Intf::Row& row, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor*);
};

//------------------------------------------------------------------------------
class AndVerb: public VerbNode
{
	VERB_DECLARE( AndVerb );
public:
	virtual int getVerbType() const { return K_AND; };
	virtual void changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext );
	virtual void accept(VerbVisitor*);
};

//------------------------------------------------------------------------------
class InVerb: public VerbNode
{
	VERB_DECLARE( InVerb );
public:
	virtual int getVerbType() const { return K_IN; };
	virtual bool preprocessQuery(	aq::tnode* pStart, aq::tnode* pNode, 
									aq::tnode* pStartOriginal );
	virtual bool changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext );
	virtual void accept(VerbVisitor*);
};

//------------------------------------------------------------------------------
class IntValueVerb: public VerbNode
{
	VERB_DECLARE( IntValueVerb );
public:
	virtual int getVerbType() const { return K_INTEGER; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual void accept(VerbVisitor*);
};

//------------------------------------------------------------------------------
class DoubleValueVerb: public VerbNode
{
	VERB_DECLARE( DoubleValueVerb );
public:
	virtual int getVerbType() const { return K_REAL; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual void accept(VerbVisitor*);
};

//------------------------------------------------------------------------------
class StringValueVerb: public VerbNode
{
	VERB_DECLARE( StringValueVerb );
public:
	virtual int getVerbType() const { return K_STRING; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual void accept(VerbVisitor*);
};

//------------------------------------------------------------------------------
class AsVerb: public VerbNode
{
	VERB_DECLARE( AsVerb );
public:
	virtual int getVerbType() const { return K_AS; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  void addResult(aq::RowProcess_Intf::Row& row, 
    VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void accept(VerbVisitor* visitor);
  const std::string& getIdent() const { return ident; }
private:
  std::string ident;
};

//------------------------------------------------------------------------------
class AsteriskVerb: public VerbNode
{
	VERB_DECLARE( AsteriskVerb );
public:
	virtual int getVerbType() const { return K_STAR; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual void accept(VerbVisitor*);
};

//------------------------------------------------------------------------------
class AscVerb: public VerbNode
{
	VERB_DECLARE( AscVerb );
public:
	virtual int getVerbType() const { return K_ASC; };
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void accept(VerbVisitor*);
};