#include "ParsException.h"
#include <aq/TreeUtilities.h>

namespace aq
{

  parsException::parsException( const char* Msg, aq::tnode* pNode, bool next )
  {
    std::ostringstream oss;
    oss << "Problem detected: Parsing: " << Msg;
    if ( pNode != nullptr )
    {
      aq::util::generate_parent( pNode, nullptr );
      std::string str;
      if ( next == false )
        syntax_tree_to_sql_form_nonext( pNode, str );
      else
        syntax_tree_to_sql_form( pNode, str );
      oss << " -> [" << str << "] <-";
    }
    this->msg = oss.str() + ".";
  }

  parsException::parsException( const char* Msg, const std::vector<std::string>& list, const std::string& name )
  {
    if ( list.size() == 0 )
      return;
    std::ostringstream oss;
    oss << "Problem detected: Parsing: " << Msg << ": " << name << " exist in defferent Tab -> ";
    for ( std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it )
    {
      if ( it != list.begin() )
        oss << " - ";
      oss << "{" << *it << "}";
    }
    this->msg += oss.str() + ".";
  }

}