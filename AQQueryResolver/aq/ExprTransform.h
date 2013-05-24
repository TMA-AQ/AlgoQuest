#ifndef __AQ_EXPRTRANSFORM_H__
#define __AQ_EXPRTRANSFORM_H__


#include "Settings.h"
#include "Column2Table.h"
#include "Base.h"
#include "Table.h"

namespace aq
{

class ExpressionTransform
{
public:
  ExpressionTransform(Base& _baseDesc, TProjectSettings& _settings);

  aq::tnode* expression_transform( aq::tnode *pNode, int *pErr );

  int get_thesaurus_for_column_reference(	Column& thesaurus, aq::tnode *pNode, int part, int *pErr );

protected:
  
  aq::tnode* transform_cmp_op( aq::tnode* pNode, int *pErr );
  aq::tnode* transform_like( aq::tnode* pNode, int *pErr );
  aq::tnode* transform_between( aq::tnode* pNode, int *pErr );

  int get_thesaurus_for_column(	
    Column& thesaurus, 
    unsigned int nTableId, 
    unsigned int nColumnId, 
    unsigned int nColumnSize,
    unsigned int nPartNumber, 
    ColumnType eColumnType, 
    int *pErr );

  int get_thesaurus_info_for_column_reference( 
    aq::tnode *pNode, 
    unsigned int *pnTableId, 
    unsigned int *pnColumnId, 
    unsigned int *pnColumnSize, 
    ColumnType *peColumnType, 
    int *pErr ); 

private:
  const Base& baseDesc;
  const TProjectSettings& settings;

};

}

#endif /* __AQ_EXPRTRANSFORM_H__ */