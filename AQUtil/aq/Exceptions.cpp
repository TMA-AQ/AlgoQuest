#include "Exceptions.h"
#include <cassert>

using namespace aq;

//------------------------------------------------------------------------------
generic_error::generic_error( EType type, const std::string& msg )
{
	this->Message = "[" + typeToString(type) + "] " + msg;
}

//------------------------------------------------------------------------------
std::string generic_error::typeToString(generic_error::EType type)
{
  switch(type)
  {
    case GENERIC: return "GENERIC"; break;
    case VERB_TYPE_MISMATCH: return "VERB_TYPE_MISMATCH"; break;
    case VERB_BAD_SYNTAX: return "VERB_BAD_SYNTAX"; break;
    case THESAURUS_NOT_FOUND: return "THESAURUS_NOT_FOUND"; break;
    case SELECT_NOT_FIRST: return "SELECT_NOT_FIRST"; break;
    case TYPE_MISMATCH: return "TYPE_MISMATCH"; break;
    case AGGREGATE_NOT_IN_SELECT_OR_HAVING: return "AGGREGATE_NOT_IN_SELECT_OR_HAVING"; break;
    case NOT_IMPLEMENED: return "NOT_IMPLEMENED"; break;
    case INVALID_BASE_FILE: return "INVALID_BASE_FILE"; break;
    case INVALID_QUERY: return "INVALID_QUERY"; break;
    case COULD_NOT_OPEN_FILE: return "COULD_NOT_OPEN_FILE"; break;
    case INVALID_TABLE: return "INVALID_TABLE"; break;
    case TABLE_ALREADY_EXISTS: return "TABLE_ALREADY_EXISTS"; break;
    case INVALID_FILE: return "INVALID_FILE"; break;
		case AQ_ENGINE: return "AQ_ENGINE"; break;
  }
}
