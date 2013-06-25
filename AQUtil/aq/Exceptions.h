#pragma once

#include <stdexcept>
#include <string>

namespace aq
{

//------------------------------------------------------------------------------
class generic_error: public std::exception
{
public:
	enum EType
	{
		GENERIC = 0,
		VERB_TYPE_MISMATCH,
		VERB_BAD_SYNTAX,
		THESAURUS_NOT_FOUND,
		SELECT_NOT_FIRST,
		TYPE_MISMATCH,
		AGGREGATE_NOT_IN_SELECT_OR_HAVING,
		NOT_IMPLEMENED,
		INVALID_BASE_FILE,
		INVALID_QUERY,
		COULD_NOT_OPEN_FILE,
		INVALID_TABLE,
		TABLE_ALREADY_EXISTS,
		AQ_ENGINE,
		INVALID_FILE
	};
	generic_error( EType type, const std::string& msg );
	generic_error( EType type, const char * format, ...);
  virtual ~generic_error() throw () {}
	virtual const char* what() const throw() { return this->Message.c_str(); };
	EType getType() const { return type; }
	static std::string typeToString(EType type);
protected:
	std::string Message;
	EType type;
};

}
