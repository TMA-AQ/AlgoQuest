#pragma once

#include "VerbNode.h"
#include <aq/DBTypes.h>

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
class ComparisonVerb: public VerbNode
{
	VERB_DECLARE( ComparisonVerb );
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

	virtual void setBaseDesc(Base * baseDesc) 
	{ 
		m_baseDesc = baseDesc;
	}

	virtual void setSettings(TProjectSettings * settings) 
	{
		m_settings = settings;
	}

private:
	Base * m_baseDesc;
	TProjectSettings * m_settings;
	virtual bool compare(	ColumnItem* item1, 
							ColumnItem* item2, 
							aq::ColumnType type );
  std::string value;
};

//------------------------------------------------------------------------------
class EqVerb: public ComparisonVerb
{
	VERB_DECLARE( EqVerb );
public:
	virtual int getVerbType() const { return K_EQ; };
private:
	bool compare(	ColumnItem* item1, 
					ColumnItem* item2, 
					aq::ColumnType type );
  virtual void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
class JeqVerb: public ComparisonVerb
{
	VERB_DECLARE( JeqVerb );
public:
	virtual int getVerbType() const { return K_JEQ; };
  virtual void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
class JautoVerb: public ComparisonVerb
{
	VERB_DECLARE( JautoVerb );
public:
	virtual int getVerbType() const { return K_JAUTO; };
};

//------------------------------------------------------------------------------
class LtVerb: public ComparisonVerb
{
	VERB_DECLARE( LtVerb );
public:
	virtual int getVerbType() const { return K_LT; };
private:
	bool compare(	ColumnItem* item1, 
					ColumnItem* item2, 
					aq::ColumnType type );
};

//------------------------------------------------------------------------------
class LeqVerb: public ComparisonVerb
{
	VERB_DECLARE( LeqVerb );
public:
	virtual int getVerbType() const { return K_LEQ; };
private:
	bool compare(	ColumnItem* item1, 
					ColumnItem* item2, 
					aq::ColumnType type );
};

//------------------------------------------------------------------------------
class GtVerb: public ComparisonVerb
{
	VERB_DECLARE( GtVerb );
public:
	virtual int getVerbType() const { return K_GT; };
private:
	bool compare(	ColumnItem* item1, 
					ColumnItem* item2, 
					aq::ColumnType type );
};

//------------------------------------------------------------------------------
class GeqVerb: public ComparisonVerb
{
	VERB_DECLARE( GeqVerb );
public:
	virtual int getVerbType() const { return K_GEQ; };
private:
	bool compare(	ColumnItem* item1, 
					ColumnItem* item2, 
					aq::ColumnType type );
};

//------------------------------------------------------------------------------
class BetweenVerb: public ComparisonVerb
{
	VERB_DECLARE( BetweenVerb );
public:
	virtual int getVerbType() const { return K_BETWEEN; };
};

//------------------------------------------------------------------------------
class NotBetweenVerb: public ComparisonVerb
{
	VERB_DECLARE( NotBetweenVerb );
public:
	virtual int getVerbType() const { return K_NOT_BETWEEN; };
};

//------------------------------------------------------------------------------
class LikeVerb: public ComparisonVerb
{
	VERB_DECLARE( LikeVerb );
public:
	virtual int getVerbType() const { return K_LIKE; };
};

//------------------------------------------------------------------------------
class NotLikeVerb: public ComparisonVerb
{
	VERB_DECLARE( NotLikeVerb );
public:
	virtual int getVerbType() const { return K_NOT_LIKE; };
};

//------------------------------------------------------------------------------
class JinfVerb: public ComparisonVerb
{
	VERB_DECLARE( JinfVerb );
public:
	virtual int getVerbType() const { return K_JINF; };
};

//------------------------------------------------------------------------------
class JieqVerb: public ComparisonVerb
{
	VERB_DECLARE( JieqVerb );
public:
	virtual int getVerbType() const { return K_JIEQ; };
};

//------------------------------------------------------------------------------
class JsupVerb: public ComparisonVerb
{
	VERB_DECLARE( JsupVerb );
public:
	virtual int getVerbType() const { return K_JSUP; };
};

//------------------------------------------------------------------------------
class JseqVerb: public ComparisonVerb
{
	VERB_DECLARE( JseqVerb );
public:
	virtual int getVerbType() const { return K_JSEQ; };
};

//------------------------------------------------------------------------------
class NeqVerb: public ComparisonVerb
{
	VERB_DECLARE( NeqVerb );
public:
	virtual int getVerbType() const { return K_NEQ; };
private:
	bool compare(	ColumnItem* item1, 
					ColumnItem* item2, 
					aq::ColumnType type );
};

//------------------------------------------------------------------------------
class JneqVerb: public ComparisonVerb
{
	VERB_DECLARE( JneqVerb );
public:
	virtual int getVerbType() const { return K_JNEQ; };
};

//------------------------------------------------------------------------------
class IsVerb: public VerbNode
{
	VERB_DECLARE( IsVerb );
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