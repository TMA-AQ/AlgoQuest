#include "ComparisonVerbs.h"
#include "VerbVisitor.h"
#include <aq/ExprTransform.h>
#include <aq/Exceptions.h>
#include <aq/Logger.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <aq/FileMapper.h>

using namespace std;

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
bool ComparisonVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
									VerbResult::Ptr resLeft, 
									VerbResult::Ptr resRight, 
									VerbResult::Ptr resNext )
{
	assert( pNode );
	assert( pNode->getDataType() != aq::tnode::tnodeDataType::NODE_DATA_STRING );
	int pErr = 0;
	//the argument given to expression_transform will be destroyed if the function
	//is successful
	aq::tnode* pNodeClone = pNode->clone_subtree();
	aq::tnode* newNode = pNodeClone;
	if( this->Context == K_WHERE )
	{
		boost::posix_time::ptime begin(boost::posix_time::microsec_clock::local_time());
                aq::ExpressionTransform expTrans(*this->m_baseDesc, *this->m_settings);
		newNode = expTrans.transform<FileMapper>(pNodeClone);
		boost::posix_time::ptime end(boost::posix_time::microsec_clock::local_time());
		std::ostringstream oss;
		oss << "expression_transform elapsed time: " << (end - begin) << " ms";
		aq::Logger::getInstance().log(AQ_INFO, "%s\n", oss.str().c_str());
	}
	
	if( newNode == pNodeClone ) //expression not transformed
	{
		delete pNodeClone ;
		if( !pNode->left || !pNode->right )
			throw verb_error(generic_error::GENERIC, this->getVerbType());
		if (this->Context == K_WHERE && 
        (((pNode->left->tag == K_PERIOD) && (pNode->right->tag == K_PERIOD)) ||
         ((pNode->left->tag == K_PERIOD) && resRight && (resRight->getType() == VerbResult::SCALAR)) ||
         ((pNode->right->tag == K_PERIOD) && resLeft && (resLeft->getType() == VerbResult::SCALAR)) ||
         (((pNode->left->tag == K_INNER) || (pNode->left->tag == K_OUTER)) && ((pNode->right->tag == K_INNER) || (pNode->right->tag == K_OUTER)))))
			return true; //comparison can be handled by the engine
		else
		{
			pNode->tag = K_DELETED;
			return false; //comparison cannot be handled by the engine
		}
	}
	if( !newNode || pErr != 0 ) //error
	{
		if( newNode )
			delete newNode ;
		delete pNodeClone ;
		throw verb_error(generic_error::GENERIC, this->getVerbType());
	}
	//pNodeClone already deleted by expression_transform
	assert( newNode->getDataType() != aq::tnode::tnodeDataType::NODE_DATA_STRING );
	*pNode = *newNode; // FIXME
  pNode->left = newNode->left;
  pNode->right = newNode->right;
  delete newNode;
	return true;
}

//------------------------------------------------------------------------------
void ComparisonVerb::changeResult(	Table::Ptr table, 
									VerbResult::Ptr resLeft, 
									VerbResult::Ptr resRight, 
									VerbResult::Ptr resNext )
{
  assert(false);
}

//------------------------------------------------------------------------------
void ComparisonVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
void EqVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
void JeqVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
bool IsVerb::preprocessQuery(	aq::tnode* pStart, aq::tnode* pNode, 
								aq::tnode* pStartOriginal )
{

	if (!pNode->right || !((pNode->right->tag == K_NOT && pNode->right->left && pNode->right->left->tag == K_NULL) || (pNode->right->tag == K_NULL)) ) // FIXME
		throw generic_error(generic_error::NOT_IMPLEMENTED, "");
	if( pNode->right->tag == K_NOT && (pNode->right->left->tag == K_NULL) )
		this->IsNot = true;
	return false;
}

//------------------------------------------------------------------------------
void IsVerb::changeResult(	Table::Ptr table, 
							VerbResult::Ptr resLeft, 
							VerbResult::Ptr resRight, 
							VerbResult::Ptr resNext )
{
  assert(false);
}

//------------------------------------------------------------------------------
void IsVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

}
}
