#ifndef __AQ_VERB_H__
#define __AQ_VERB_H__

#include <aq/Base.h>
#include <aq/Settings.h>

#include "VerbResult.h"

#include <aq/Object.h>
#include <aq/RowProcess_Intf.h>
#include <aq/Table.h>
#include <aq/parser/SQLParser.h>
#include <aq/parser/sql92_grm_tab.hpp>
#include <aq/Exceptions.h>

namespace aq {
namespace verb {

// forward declaration
class VerbVisitor;

//------------------------------------------------------------------------------
class verb_error: public aq::generic_error
{
public:
	verb_error( EType type, int verbTag );
};
//--------------------------------------------------------------------------
class Verb: public Object<Verb>
{
public:
	//functions that need implementation

	//return verb identifier (K_XXX from sql92_grm_tab.h)
	virtual int getVerbType() const = 0;

	/// preprocess query to gather information about it before it is changed
	/// or change the aq::tnode subtree before the verb tree is built based on it
	/// 1. If the subtree can be solved by the engine, return true.
	/// 2. If the subtree will be solved by the verb return false.
	/// pStart - top of the query (SELECT node)
	/// pNode - node to which this verb corresponds
	/// Be carefull ! This method is called in top-down order, in the verb tree,
	/// as the verb tree is being built
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal ) = 0;
  //{ 
  //  return false; 
  //}

	/// make changes to the query before it is executed
	/// results that should be passed on to parent verbs should be placed in Result
	/// 1. If verb can be solved and changeResult is not necessary, pNode should
	/// be assigned the appropriate subtree and changeQuery should return true.
	/// 2. If verb cannot be solved completely changeQuery should return false.
	/// pStart/pNode - same as preprocessQuery
	/// parameters - array of results from children verbs
	/// Be carefull ! This method is called in bottom-up order, in the verb tree
	virtual bool changeQuery( aq::tnode* pStart, aq::tnode* pNode, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext ) = 0;
  //{ 
  //  return false; 
  //}

	/// make changes to the table resulted from executing the query
	/// results that should be passed on to parent verbs should be placed in Result
	/// table - should be read by leaf nodes and written by top level nodes
	/// parameters - same as changeQuery
	/// Be carefull ! This method is called in bottom-up order, in the verb tree
	virtual void changeResult( Table::Ptr table, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext ) = 0;
  //{
  //}

  /// Apply the verb on the row
  /// This method is called in bottom-up order in the verb tree
  virtual void addResult ( aq::Row& row ) = 0;
  //{
  //}

  /// Set the Base Description
  virtual void setBaseDesc(Base * BaseDesc) 
  { 
    // empty by default
  }
  
  /// Set the Base Description
  virtual void setSettings(Settings * settings) 
  {
    // empty by default
  }

	//functions that do not need implementation
	void setContext( int context );
	VerbResult::Ptr getResult() const;

	// Visitor accept
	virtual void accept(VerbVisitor*) = 0;

  //FIXME: debug13 ugly hack used so that FirstValue verb can use LagVerb protected:
	//the result is automatically passed as a parameter to parent verbs
	//(in VerbNode)
	VerbResult::Ptr Result;

	//the context indicates to what major category the verb is in
	//(behavior should be different depending on context)
	//can be K_SELECT, K_WHERE, K_HAVING ..
	int Context;
	bool Disabled; //if true, do not use this Verb or its children anymore

	//irrelevant when implementing derived class
public:
	Verb();
	virtual ~Verb(){};
};

}
}

#endif