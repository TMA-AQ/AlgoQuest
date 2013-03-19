#pragma once

#include <stdexcept>
#include <string>

//------------------------------------------------------------------------------
class generic_error: public std::exception
{
public:
	enum EType
	{
		GENERIC,
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
	virtual const char* what() const throw() { return this->Message.c_str(); };
	EType getType() const { return type; }
	static std::string typeToString(EType type);
protected:
	std::string Message;
	EType type;
};

//------------------------------------------------------------------------------
class verb_error: public generic_error
{
public:
	verb_error( EType type, int verbTag );
};