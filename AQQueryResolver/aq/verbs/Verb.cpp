#include "Verb.h"
#include "VerbFactory.h"
#include "VerbVisitor.h"
#include <aq/parser/ID2Str.h>
#include <aq/Logger.h>

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
verb_error::verb_error( EType type, int verbTag ):
	generic_error( type, "" )
{
	this->Message +=  id_to_string( verbTag );
}

//------------------------------------------------------------------------------
Verb::Verb(): Result(nullptr), context(0), disabled(false)
{
}

//------------------------------------------------------------------------------
Verb::~Verb()
{
}

//------------------------------------------------------------------------------
void Verb::setContext(tnode::tag_t _context)
{
	this->context = context;
}

//------------------------------------------------------------------------------
VerbResult::Ptr Verb::getResult() const
{
	return this->Result;
}

//------------------------------------------------------------------------------
void Verb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

}
}