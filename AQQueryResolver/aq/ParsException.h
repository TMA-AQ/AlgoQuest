#ifndef __PARSEXCEPTION_H__
# define __PARSEXCEPTION_H__

# include <iostream>
# include <sstream>
# include <exception>
# include <vector>
# include <string>
# include <aq/parser/SQLParser.h>

namespace aq
{

  class parsException : public std::exception
  {
  public:
    parsException( const char* Msg, aq::tnode* pNode = nullptr, bool next = false );
    parsException( const char* Msg, const std::vector<std::string>& list, const std::string& name );

    virtual ~parsException() throw() {}

    virtual const char * what() const throw()
    {
      return this->msg.c_str();
    }

  private:
    std::string msg;
  };

}

#endif // __PARSEXECPTION_H__