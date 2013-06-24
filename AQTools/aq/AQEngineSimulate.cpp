#include "AQEngineSimulate.h"

#include <aq/TreeUtilities.h>

namespace aq
{

  AQEngineSimulate::AQEngineSimulate(aq::Base& _baseDesc, aq::TProjectSettings& _settings)
    : baseDesc(_baseDesc), settings(_settings)
  {
    srand( static_cast<unsigned int>( time( NULL ) ) ); //temporaire
  }


  void AQEngineSimulate::call(aq::tnode * pNode, aq::AQEngine_Intf::mode_t mode, int selectLevel)
  {
    //
    // Get prefix form of query.
    std::string str;

    aq::generate_parent( pNode, NULL );
    aq::syntax_tree_to_prefix_form( pNode, str );

    aq::Logger::getInstance().log(AQ_INFO, "---\n");
    aq::Logger::getInstance().log(AQ_INFO, "Get prefix form of query\n");
    aq::Logger::getInstance().log(AQ_INFO, "%s\n", str.c_str());

    //
    // Process with parse jeq
    ParseJeq(str);

    aq::Logger::getInstance().log(AQ_INFO, "---\n");
    aq::Logger::getInstance().log(AQ_INFO, "Get prefix form of query after jeq parser\n");
    aq::Logger::getInstance().log(AQ_INFO, "%s\n", str.c_str());

    this->aqMatrix.reset(new aq::AQMatrix(this->settings));

    this->createTableIDs( pNode );

    std::cout << "TableIDs value =" << std::endl;
    std::for_each(this->tableIDs.begin(), this->tableIDs.end(), [&] (long long v) {
      std::cout << "[" << v << "]" << std::endl;
    });

    this->aqMatrix->simulate( rand() % 1000, this->tableIDs );

  }

  void AQEngineSimulate::setAQMatrix(boost::shared_ptr<aq::AQMatrix> _aqMatrix)
  {
    this->aqMatrix = _aqMatrix;
  }

  void AQEngineSimulate::setTablesIDs(std::vector<llong>& _tableIDs)
  {
    this->tableIDs.resize(_tableIDs.size());
    std::copy(_tableIDs.begin(), _tableIDs.end(), this->tableIDs.begin());
  }

  boost::shared_ptr<aq::AQMatrix> AQEngineSimulate::getAQMatrix()
  {
    return this->aqMatrix;
  }

  const std::vector<llong>& AQEngineSimulate::getTablesIDs() const
  {
    return this->tableIDs;
  }

  void  AQEngineSimulate::createTableIDs(aq::tnode* pNode)
  {
    if ( !pNode )
      return;

    if ( pNode->tag == K_IDENT 
      && std::find( this->tableIDs.begin(), this->tableIDs.end(), this->baseDesc.getTable( pNode->getData().val_str )->ID ) == this->tableIDs.end() )
      this->tableIDs.push_back( this->baseDesc.getTable( pNode->getData().val_str )->ID );

    this->createTableIDs( pNode->left );
    this->createTableIDs( pNode->right );
    this->createTableIDs( pNode->next );
  }
}