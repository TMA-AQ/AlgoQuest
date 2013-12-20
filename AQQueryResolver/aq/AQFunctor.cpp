#include "AQFunctor.h"

#include <aq\parser\sql92_grm_tab.hpp>
#include <aq\TreeUtilities.h>

namespace aq
{

  AQFunctor::AQFunctor(aq::tnode *node, const std::string& path)
  {
    try
    {
      this->_lib = LoadLibrary(path.c_str());
      if (this->_lib == nullptr)
      {
        DWORD rc = GetLastError();
        wprintf(L"Format message failed with %d\n", GetLastError());
      }
      this->assignFunctor(node);
    }
    catch (...)
    {
      FreeLibrary(this->_lib);
      throw;
    }
  }

  AQFunctor::~AQFunctor()
  {
    FreeLibrary(this->_lib);
  }

  void AQFunctor::assignFunctor(aq::tnode *node)
  {
    try
    {
      if (!node || (node->tag == K_SELECT && !node->left))
        throw;
      aq::util::generate_parent(node, NULL);
      vectorNode listSave;
      node->find_nodes(K_FUNC, listSave);
      for (auto& it : listSave)
      {
        this->createFunctor(it);
      }
    }
    catch (...)
    {
      FreeLibrary(this->_lib);
      throw;
    }
  }

  void AQFunctor::createFunctor(aq::tnode *node)
  {
    try
    {
      if (!node || node->tag != K_FUNC)
        throw;
      vectorString  arg;
      std::string   ident = node->getData().val_str;
      aq::tnode*  pNode = node->left->clone_subtree();

      while (pNode)
      {
        aq::tnode* tmp;
        if (pNode->tag == K_COMMA && pNode->left)
          tmp = pNode->left;
        else if (pNode->tag == K_COMMA && !pNode->left)
          throw;
        else
          tmp = pNode;
        std::ostringstream oss;
        if (tmp->getDataType() == aq::tnode::NODE_DATA_STRING)
          arg.push_back(tmp->getData().val_str);
        else if (tmp->getDataType() == aq::tnode::NODE_DATA_INT)
        {
          oss << tmp->getData().val_int;
          arg.push_back(oss.str());
        }
        else
        {
          oss << tmp->getData().val_number;
          arg.push_back(oss.str());
        }
        pNode = pNode->right;
      }
      this->_funtor.push_back(new dataFunctor(arg, ident, this->_lib));
    }
    catch (...)
    {
      FreeLibrary(this->_lib);
      throw;
    }
  }

  void AQFunctor::dump(std::ostream& os)
  {
    for (auto& it : this->_funtor)
      it->dump(os);
  }

  void AQFunctor::callFunctor()
  {
    for (auto& it : this->_funtor)
      it->callFunc();
  }

  void AQFunctor::callFunctor(const std::string& ident)
  {
    for (auto& it : this->_funtor)
      if (it->getIdent() == ident)
        it->callFunc();
  }

  void AQFunctor::setArgChanged()
  {
    
  }

  void AQFunctor::setArgChanged(const std::string& ident)
  {
     
  }

}