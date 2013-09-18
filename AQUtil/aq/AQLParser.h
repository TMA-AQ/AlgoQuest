#ifndef __AQ_PARSER_H__
#define __AQ_PARSER_H__

#include "AQLQuery.h"

namespace aq {
namespace parser {

bool parse(const std::string& queryStr, aq::core::SelectStatement& query);

}
}

#endif