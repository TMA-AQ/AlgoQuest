#pragma once

#include "AQMatrix.h"
#include "Settings.h"
#include <aq/AQLQuery.h>

namespace aq
{

class AQEngineCallback_Intf
{
public:
	virtual void getValue(uint64_t key, size_t packet, uint8_t*& values, size_t& size) const = 0;
};

class AQEngine_Intf
{
public:
  enum mode_t
  {
    REGULAR, 
    NESTED_1,
    NESTED_2
  };

	virtual ~AQEngine_Intf() {}
  
  virtual void prepare() const = 0;
  virtual void clean() const = 0;
  
  virtual void call(const std::string& query, mode_t mode = mode_t::REGULAR) = 0;
  virtual void call(const aq::core::SelectStatement& query, mode_t mode = mode_t::REGULAR) = 0;

  virtual void renameResult(unsigned int id, std::vector<std::pair<std::string, std::string> >& resultTables) = 0;
	virtual boost::shared_ptr<aq::AQMatrix> getAQMatrix() = 0;
	virtual const std::vector<llong>& getTablesIDs() const = 0;
};

}