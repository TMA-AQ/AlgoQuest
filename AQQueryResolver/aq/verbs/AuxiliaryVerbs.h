#pragma once

#include "VerbNode.h"

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
class ColumnVerb: public VerbNode
{
public:
  ColumnVerb();
  typedef boost::intrusive_ptr<ColumnVerb> Ptr;

	virtual int getVerbType() const { return K_PERIOD; };
	virtual bool changeQuery( aq::tnode* pStart, aq::tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void addResult(aq::Row& row);

	std::string getTableName() const;
	std::string getColumnName() const;
	std::string getColumnOnlyName() const;
  
  virtual void setBaseDesc(Base * baseDesc) 
  { 
    m_baseDesc = baseDesc;
  }
  
  virtual void setSettings(Settings * settings) 
  {
    m_settings = settings;
  }

	virtual void accept(VerbVisitor*);

private:
	std::string TableName, ColumnName, ColumnOnlyName;
  Base * m_baseDesc;
  Settings * m_settings;
  int index;
  int computed_index;
};

//------------------------------------------------------------------------------
class CommaVerb: public VerbNode
{
public:
  typedef boost::intrusive_ptr<CommaVerb> Ptr;

	virtual int getVerbType() const { return K_COMMA; };
	bool changeQuery( aq::tnode* pStart, aq::tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void addResult(aq::Row& row);
  virtual void accept(VerbVisitor*);
};

//------------------------------------------------------------------------------
class AndVerb: public VerbNode
{
public:
  typedef boost::intrusive_ptr<CommaVerb> Ptr;

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
public:
  typedef boost::intrusive_ptr<InVerb> Ptr;

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
public:
  typedef boost::intrusive_ptr<IntValueVerb> Ptr;

	virtual int getVerbType() const { return K_INTEGER; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual void accept(VerbVisitor*);
};

//------------------------------------------------------------------------------
class DoubleValueVerb: public VerbNode
{
public:
  typedef boost::intrusive_ptr<DoubleValueVerb> Ptr;

	virtual int getVerbType() const { return K_REAL; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual void accept(VerbVisitor*);
};

//------------------------------------------------------------------------------
class StringValueVerb: public VerbNode
{
public:
  typedef boost::intrusive_ptr<StringValueVerb> Ptr;

	virtual int getVerbType() const { return K_STRING; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual void accept(VerbVisitor*);
};

//------------------------------------------------------------------------------
class AsVerb: public VerbNode
{
public:
  typedef boost::intrusive_ptr<AsVerb> Ptr;
  AsVerb();

	virtual int getVerbType() const { return K_AS; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  void addResult(aq::Row& row);
	virtual void accept(VerbVisitor* visitor);
  const std::string& getIdent() const { return ident; }
private:
  std::string ident;
  int index;
};

//------------------------------------------------------------------------------
class AsteriskVerb: public VerbNode
{
public:
  typedef boost::intrusive_ptr<AsteriskVerb> Ptr;

	virtual int getVerbType() const { return K_STAR; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual void accept(VerbVisitor*);
};

//------------------------------------------------------------------------------
class AscVerb: public VerbNode
{
public:
  typedef boost::intrusive_ptr<AscVerb> Ptr;

	virtual int getVerbType() const { return K_ASC; };
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void accept(VerbVisitor*);
};

}
}