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

/// \brief Exception for Verb
class verb_error: public aq::generic_error
{
public:
	verb_error( EType type, int verbTag );
};

/// \brief Interface Verb
///
/// Behaviour of SQL Verb are described in sub-class of Verb Interface
/// Each sub-class must define five method
class Verb: public Object<Verb>
{
public:
  Verb();
	virtual ~Verb();
	
	/// \brief return verb identifier (K_XXX from sql92_grm_tab.h)
  virtual int getVerbType() const = 0;

	/// \brief preprocess query to gather information about it before it is changed
	/// or change the aq::tnode subtree before the verb tree is built based on it
	/// \param pStart top of the query
	/// \param pNode node to which this verb corresponds
  /// \return true if the subtree can be solved by the engine, false if not
  /// \note This method is called in top-down order, in the verb tree, as the verb tree is being built
	virtual bool preprocessQuery(aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal) = 0;

	/// \brief make changes to the query before it is executed
	/// results that should be passed on to parent verbs should be placed in Result
	/// \param pStart top of the query
  /// \param pNode node to which this verb corresponds
	/// \deprecated \param resLeft results from left children verbs
	/// \deprecated \param resRight results from right children verbs
	/// \deprecated \param resNext results from next children verbs
	/// \return 
  ///   - True if verb can be solved and changeResult is not necessary. Then pNode should
	///     be assigned the appropriate subtree.
	///   - False if verb cannot be solved completely changeQuery.
	/// \note This method is called in bottom-up order, in the verb tree
	virtual bool changeQuery(aq::tnode* pStart, aq::tnode* pNode, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext) = 0;

  /// \deprecated
	/// \brief make changes to the table resulted from executing the query
	/// results that should be passed on to parent verbs should be placed in Result
	/// \param table should be read by leaf nodes and written by top level nodes
	/// \param resLeft results from left children verbs
	/// \param resRight results from right children verbs
	/// \param resNext results from next children verbs
	/// \note This method is called in bottom-up order, in the verb tree
	virtual void changeResult(Table::Ptr table, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext) = 0;

  /// \brief Apply the verb on the row
  /// \param row the row resulting after processing AQ_Engine, and all bottom verb
  /// \note This method is called in bottom-up order in the verb tree
  virtual void addResult (aq::Row& row) = 0;

  /// \brief Set the Base Description
  /// \param BaseDesc the base description
  /// \todo BaseDesc should be a const shared_ptr
  virtual void setBaseDesc(Base * BaseDesc) 
  { 
    // empty by default
  }
  
  /// \brief Set the settings
  /// \param settings the query settings
  /// \todo settings should be a const shared_ptr
  virtual void setSettings(Settings * settings) 
  {
    // empty by default
  }

	/// \brief Visitor defign pattern accept method
  /// \param visitor the visitor
  /// visitor are used to perform operation on verb tree
	virtual void accept(VerbVisitor * visitor) = 0;
  
	void setContext(tnode::tag_t _context);
  void disable() { disabled = true; }

	VerbResult::Ptr getResult() const;
  tnode::tag_t getContext() const { return context; }
  bool isDisabled() const { return disabled; }  

protected:
	/// \brief the result is automatically passed as a parameter to parent verbs (in VerbNode)
  /// \fixme: ugly hack used so that FirstValue verb can use LagVerb protected:
	VerbResult::Ptr Result;
  
	/// the context indicates to what major category the verb is in
	/// (behavior should be different depending on context)
	/// can be K_SELECT, K_WHERE, K_HAVING ..
	tnode::tag_t context;
  
  /// if true, do not use this Verb or its children anymore
  bool disabled; 
};

}
}

#endif
