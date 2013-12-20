#ifndef __AQFUNCTOR_H__
#define __AQFUNCTOR_H__

#ifdef WIN32
# include <Windows.h>
#else
# define HINSTANCE void * // TODO
# define DWORD int // TODO

HINSTANCE GetProcAddress(int, const char *) { return nullptr; } // TODO
int GetLastError() { return 0; }

#endif


#include <stdio.h>
#include <ostream>
#include <iostream>

#include <aq/parser/SQLParser.h>

namespace aq
{
 
  //--------------------------------------------------------------------
  // dataFunctor is a Functor, who call a function in a dynamic library
  //--------------------------------------------------------------------

  typedef std::vector<std::string> vectorString;
  typedef std::vector<aq::tnode*> vectorNode;
  typedef int (*func_t)(const vectorString& arg);

  class dataFunctor
  {
  public:
    dataFunctor(const vectorString& arg, const std::string& ident, HINSTANCE lib)
    {
      this->_arg = arg;
      this->_ident = ident;
      try
      {
#ifdef WIN32
        this->_func = (func_t)GetProcAddress(lib, _ident.c_str());
        if (this->_func == NULL)
        {
          DWORD rc = GetLastError();
          throw;
        }
#endif
      }
      catch (...)
      {
        throw;
      }
    }

    void callFunc(/*Mettre matrix/basedesc pour récupérer le bon element peut etre*/)
    {
      try
      {
        this->_func(this->_arg);
      }
      catch (...)
      {
        return;
      }
    }

    void dump(std::ostream& os)
    {
      os << "Function call : " << this->_ident << std::endl;
      for (auto& it : this->_arg)
      {
        os << "[" << it << "]  ";
      }
      os << std::endl;
    }

    const std::string& getIdent() const
    {
      return this->_ident;
    }

    void setArg(const vectorString& arg)
    {
      this->_arg = arg;
    }

  private:
    vectorString  _arg;   ///< arg of the plugin function
    std::string   _ident; ///< name of the plugin function
    func_t        _func;  ///< the plugin function
  };
  
  //--------------------------------------------------------------------
  // AQFunctor stock every functor and create them in the constructor
  //--------------------------------------------------------------------

  typedef std::vector<dataFunctor*> vectorData;

  class AQFunctor
  {
  public:
    AQFunctor(aq::tnode *node, const std::string& path);
    ~AQFunctor();

    void assignFunctor(aq::tnode *node);
    void createFunctor(aq::tnode *node);
         
    void setArgChanged(/* ?? (changer la valeur d'un champs par exemple) */);
    void setArgChanged(const std::string& ident);
         
    void callFunctor();
    void callFunctor(const std::string& ident);
         
    void dump(std::ostream& os);

  private:
    vectorData  _funtor;
    HINSTANCE   _lib; ///< plugin handler
  };

}

#endif // __AQFUNCTOR_H__
