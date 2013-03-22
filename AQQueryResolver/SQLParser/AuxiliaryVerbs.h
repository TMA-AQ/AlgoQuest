#pragma once

#include "Verb.h"

//------------------------------------------------------------------------------
class ColumnVerb: public Verb
{
	VERB_DECLARE( ColumnVerb );
public:
	virtual int getVerbType() const { return K_PERIOD; };
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
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
class CommaVerb: public Verb
{
	VERB_DECLARE( CommaVerb );
public:
	virtual int getVerbType() const { return K_COMMA; };
	bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void accept(VerbVisitor*);
};

//------------------------------------------------------------------------------
class AndVerb: public Verb
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
class InVerb: public Verb
{
	VERB_DECLARE( InVerb );
public:
	virtual int getVerbType() const { return K_IN; };
	virtual bool preprocessQuery(	tnode* pStart, tnode* pNode, 
									tnode* pStartOriginal );
	virtual bool changeQuery(	tnode* pStart, tnode* pNode,
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext );
};

//------------------------------------------------------------------------------
class IntValueVerb: public Verb
{
	VERB_DECLARE( IntValueVerb );
public:
	virtual int getVerbType() const { return K_INTEGER; };
	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
};

//------------------------------------------------------------------------------
class DoubleValueVerb: public Verb
{
	VERB_DECLARE( DoubleValueVerb );
public:
	virtual int getVerbType() const { return K_REAL; };
	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
};

//------------------------------------------------------------------------------
class StringValueVerb: public Verb
{
	VERB_DECLARE( StringValueVerb );
public:
	virtual int getVerbType() const { return K_STRING; };
	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
};

//------------------------------------------------------------------------------
class AsVerb: public Verb
{
	VERB_DECLARE( AsVerb );
public:
	virtual int getVerbType() const { return K_AS; };
	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};

//------------------------------------------------------------------------------
class AsteriskVerb: public Verb
{
	VERB_DECLARE( AsteriskVerb );
public:
	virtual int getVerbType() const { return K_STAR; };
	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
};

//------------------------------------------------------------------------------
class AscVerb: public Verb
{
	VERB_DECLARE( AscVerb );
public:
	virtual int getVerbType() const { return K_ASC; };
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};