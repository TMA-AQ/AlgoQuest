#pragma once

#include "VerbNode.h"
#include <aq/DBTypes.h>

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
class ComparisonVerb: public VerbNode
{
public:
	virtual int getVerbType() const { return -1; };
	virtual bool changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext );
	virtual void changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor* visitor);
  
  const std::string& getValue() const { return value; }

	virtual void setBaseDesc(Base::Ptr baseDesc) 
	{ 
		m_baseDesc = baseDesc;
	}

	virtual void setSettings(Settings::Ptr settings) 
	{
		m_settings = settings;
	}

private:
	Base::Ptr m_baseDesc;
	Settings::Ptr m_settings;
	//virtual bool compare(	ColumnItem* item1, 
	//						ColumnItem* item2, 
	//						aq::ColumnType type );
  std::string value;
};

//------------------------------------------------------------------------------
class EqVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_EQ; };
private:
	//bool compare(	ColumnItem* item1, 
	//				ColumnItem* item2, 
	//				aq::ColumnType type );
  virtual void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
class JeqVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_JEQ; };
  virtual void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
class JautoVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_JAUTO; };
};

//------------------------------------------------------------------------------
class LtVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_LT; };
private:
	//bool compare(	ColumnItem* item1, 
	//				ColumnItem* item2, 
	//				aq::ColumnType type );
};

//------------------------------------------------------------------------------
class LeqVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_LEQ; };
private:
	//bool compare(	ColumnItem* item1, 
	//				ColumnItem* item2, 
	//				aq::ColumnType type );
};

//------------------------------------------------------------------------------
class GtVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_GT; };
private:
	//bool compare(	ColumnItem* item1, 
	//				ColumnItem* item2, 
	//				aq::ColumnType type );
};

//------------------------------------------------------------------------------
class GeqVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_GEQ; };
private:
	//bool compare(	ColumnItem* item1, 
	//				ColumnItem* item2, 
	//				aq::ColumnType type );
};

//------------------------------------------------------------------------------
class BetweenVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_BETWEEN; };
};

//------------------------------------------------------------------------------
class NotBetweenVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_NOT_BETWEEN; };
};

//------------------------------------------------------------------------------
class LikeVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_LIKE; };
};

//------------------------------------------------------------------------------
class NotLikeVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_NOT_LIKE; };
};

//------------------------------------------------------------------------------
class JinfVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_JINF; };
};

//------------------------------------------------------------------------------
class JieqVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_JIEQ; };
};

//------------------------------------------------------------------------------
class JsupVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_JSUP; };
};

//------------------------------------------------------------------------------
class JseqVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_JSEQ; };
};

//------------------------------------------------------------------------------
class NeqVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_NEQ; };
private:
	//bool compare(	ColumnItem* item1, 
	//				ColumnItem* item2, 
	//				aq::ColumnType type );
};

//------------------------------------------------------------------------------
class JneqVerb: public ComparisonVerb
{
public:
	virtual int getVerbType() const { return K_JNEQ; };
};

//------------------------------------------------------------------------------
class IsVerb: public VerbNode
{
public:
	virtual int getVerbType() const { return K_IS; };
	virtual bool preprocessQuery(	aq::tnode* pStart, aq::tnode* pNode, 
									aq::tnode* pStartOriginal );
	virtual void changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor* visitor);
private:
	bool IsNot;
};

}
}