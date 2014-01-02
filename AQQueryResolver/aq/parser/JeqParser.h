#pragma once

#include "SQLParser.h"
#include "sql92_grm_tab.hpp"
#include <string>
#include <vector>

namespace aq {
namespace parser {

static const unsigned int nrJoinTypes = 7;
static const aq::tnode::tag_t joinTypes[]    = { K_JEQ, K_JAUTO, K_JNEQ, K_JINF, K_JIEQ, K_JSUP, K_JSEQ };
static const aq::tnode::tag_t inverseTypes[] = { K_JEQ, K_JAUTO, K_JNEQ, K_JSUP, K_JSEQ, K_JINF, K_JIEQ };

///
std::vector<std::string> ParseJeq(std::string& aql_query, bool add_active_neutral_filter = false);

}
}
