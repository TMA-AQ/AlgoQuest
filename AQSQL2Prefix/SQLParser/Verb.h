#pragma once

#include "SQLParser.h"
#include "Table.h"
#include "sql92_grm_tab.h"
#include <aq/Exceptions.h>

// forward declaration
class VerbVisitor;

//------------------------------------------------------------------------------
class verb_error: public aq::generic_error
{
public:
	verb_error( EType type, int verbTag );
};
//--------------------------------------------------------------------------
class Verb: public Object
{
public:
	//functions that need implementation

	//return verb identifier (K_XXX from sql92_grm_tab.h)
	virtual int getVerbType() const = 0;

	//preprocess query to gather information about it before it is changed
	//or change the tnode subtree before the verb tree is built based on it
	//1. If the subtree can be solved by the engine, return true.
	//2. If the subtree will be solved by the verb return false.
	//pStart - top of the query (SELECT node)
	//pNode - node to which this verb corresponds
	//attention! the method is called in top-down order, in the verb tree,
	//as the verb tree is being built
	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal ){ return false; }

	//make changes to the query before it is executed
	//results that should be passed on to parent verbs should be placed in Result
	//1. If verb can be solved and changeResult is not necessary, pNode should
	//be assigned the appropriate subtree and changeQuery should return true.
	//2. If verb cannot be solved completely changeQuery should return false.
	//pStart/pNode - same as preprocessQuery
	//parameters - array of results from children verbs
	//attention! the method is called in bottom-up order, in the verb tree
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext ){ return false; }

	//make changes to the table resulted from executing the query
	//results that should be passed on to parent verbs should be placed in Result
	//table - should be read by leaf nodes and written by top level nodes
	//parameters - same as changeQuery
	//attention! the method is called in bottom-up order, in the verb tree
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext ){}

  /// Set the Base Description
  virtual void setBaseDesc(Base * BaseDesc) 
  { 
    // empty by default
  }
  
  /// Set the Base Description
  virtual void setSettings(TProjectSettings * settings) 
  {
    // empty by default
  }

	//functions that do not need implementation
	void setContext( int context );
	VerbResult::Ptr getResult() const;

	// Visitor accept
	virtual void accept(VerbVisitor*);

//debug13 ugly hack used so that FirstValue verb can use LagVerb protected:
	//the result is automatically passed as a parameter to parent verbs
	//(in VerbNode)
	VerbResult::Ptr Result;

	//the context indicates to what major category the verb is in
	//(behavior should be different depending on context)
	//can be K_SELECT, K_WHERE, K_HAVING ..
	int Context;
	bool Disabled; //if true, do not use this Verb or its children anymore

	//irrelevant when implementing derived class
private:
	OBJECT_DECLARE( Verb );
public:
	Verb();
	virtual ~Verb(){};
	virtual Verb* clone() const = 0;
};

//--------------------------------------------------------------------------
class VerbNode: public Object
{
	OBJECT_DECLARE( VerbNode );
public:
	VerbNode();
	bool build( tnode* pStart, tnode* pNode, tnode* pStartOriginal, int context, Base& BaseDesc, TProjectSettings& settings );
	void setLeftChild( VerbNode::Ptr child );
	void setRightChild( VerbNode::Ptr child );
	void setBrother( VerbNode::Ptr brother );

	//true - executed and done (the node should be deleted)
	void changeQuery();
	void changeResult( Table::Ptr table );
	
	void accept(VerbVisitor*);

	Verb::Ptr getVerbObject();
	VerbNode::Ptr getLeftChild();
	VerbNode::Ptr getRightChild();
	VerbNode::Ptr getBrother();
private:
	Verb::Ptr VerbObject;
	tnode* pStart; //top node in the query tree
	tnode* pNode; //tnode to which this VerbNode corresponds
	std::vector<VerbNode*> Parents;
	VerbNode::Ptr Left, Right, Brother;
};

//--------------------------------------------------------------------------
class VerbFactory
{
public:
	void addVerb( Verb::Ptr verb );
	Verb::Ptr getVerb( int verbType ) const;
	static VerbFactory& GetInstance();
private:
	//disable creation
	VerbFactory(){};
	VerbFactory(const VerbFactory& source){};
	VerbFactory& operator=( const VerbFactory& source ){ return *this; }

	std::vector<Verb::Ptr> Verbs;
};

//------------------------------------------------------------------------------
//macro
#define VERB_DECLARE( class_name )\
	OBJECT_DECLARE( class_name );\
	private:\
	class FactoryRegister##class_name\
	{\
	public:\
		FactoryRegister##class_name()\
		{\
			VerbFactory::GetInstance().addVerb( new class_name() );\
		}\
	};\
	static FactoryRegister##class_name FactoryRegisterObj##class_name;\
	public:\
	class_name();\
	virtual Verb* clone() const { return new class_name(*this); }

//------------------------------------------------------------------------------
#define VERB_IMPLEMENT( class_name )\
	class_name::FactoryRegister##class_name class_name::FactoryRegisterObj##class_name;


/*
//--------------------------------------------------------------------------
class Select: public Verb
{
	VERB_DECLARE( Select );
/*	OBJECT_DECLARE( Select );
private:
	class FactoryRegisterSelect
	{
	public:
	FactoryRegisterSelect()
	{
	VerbFactory::GetInstance().addVerb( new Select() );
	}
	};
	static FactoryRegisterSelect FactoryRegisterObjSelect;
public:
	Select();
	virtual Verb* clone() const { return new Select(*this); }
public:
	virtual int getVerbType() const{ return K_SELECT; };
	virtual bool changeQuery( tnode* pStart, tnode*& pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext ){ return false; };
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext ){};
};*/