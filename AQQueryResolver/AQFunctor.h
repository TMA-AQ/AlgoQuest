#ifndef __AQFUNCTOR_H__
# define __AQFUNCTOR_H__

# include <Windows.h>
# include <stdio.h>
# include <ostream>
# include <iostream>

# include <aq\parser\SQLParser.h>

namespace aq
{
 
  //--------------------------------------------------------------------
  // dataFunctor is a Functor, who call a function in a dynamique library
  //--------------------------------------------------------------------

  typedef std::vector<std::string>  vectorString;
  typedef std::vector<aq::tnode*>   vectorNode;
  typedef int                       (*func_t)(const vectorString& arg);

  class         dataFunctor
  {
  public:
    dataFunctor(const vectorString& arg, const std::string& ident, HINSTANCE lib)
    {
      this->_arg = arg;
      this->_ident = ident;
      try
      {
        this->_func = (func_t)GetProcAddress(lib, _ident.c_str());
        if (this->_func == NULL)
        {
          DWORD rc = GetLastError();
          throw;
        }
      }
      catch (...)
      {
        throw;
      }
    }

    void          callFunc(/*Mettre matrix/basedesc pour récupérer le bon element peut etre*/)
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

    void          dump(std::ostream& os)
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

    void          setArg(const vectorString& arg)
    {
      this->_arg = arg;
    }

  private:
    // arg of the function
    vectorString  _arg;
    // name of the function
    std::string   _ident;
    // the function
    func_t        _func;
  };
  
  //--------------------------------------------------------------------
  // AQFunctor stock every functor and create them in the constructor
  //--------------------------------------------------------------------

  typedef std::vector<dataFunctor*>       vectorData;

  class         AQFunctor
  {
  public:
    AQFunctor(aq::tnode *node, const std::string& path);
    ~AQFunctor();

    void        assignFunctor(aq::tnode *node);
    void        createFunctor(aq::tnode *node);

    void        setArgChanged(/* ?? (changer la valeur d'un champs par exemple) */);
    void        setArgChanged(const std::string& ident);

    void        callFunctor();
    void        callFunctor(const std::string& ident);

    void        dump(std::ostream& os);

  private:
    vectorData  _funtor;
    // the library
    HINSTANCE   _lib;
  };

}

#endif // __AQFUNCTOR_H__
