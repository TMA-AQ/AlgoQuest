#pragma once

#include "VerbNode.h"
#include <aq/DBTypes.h>

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
class CastVerb: public VerbNode
{
public:
	virtual int getVerbType() const { return K_CAST; };

	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual bool changeQuery( aq::tnode* pStart, aq::tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor* visitor);
private:
	void solve( VerbResult::Ptr resLeft );
	aq::ColumnType ConvertType;
};

//------------------------------------------------------------------------------
class NvlVerb: public VerbNode
{
public:
	virtual int getVerbType() const { return K_NVL; };

	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
class DecodeVerb: public VerbNode
{
public:
	virtual int getVerbType() const { return K_DECODE; };

	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor* visitor);
};

}
}