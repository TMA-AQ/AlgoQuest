#pragma once

#include <string>
#include <vector>

namespace aq
{

  ///
  std::vector<std::string> ParseJeq(std::string& aql_query, bool add_active_neutral_filter = false);

}