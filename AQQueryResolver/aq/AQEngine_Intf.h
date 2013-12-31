#ifndef __AQ_ENGINE_INTF_H__
#define __AQ_ENGINE_INTF_H__

#if defined (WIN32)
# ifdef AQENGINE_EXPORTS
#  define AQENGINE_API __declspec(dllexport)
# else
#  define AQENGINE_API __declspec(dllimport)
# endif
#else
# define AQLIB_API __stdcall
#endif

#include "AQMatrix.h"
#include <aq/AQLQuery.h>

namespace aq
{

class AQENGINE_API AQEngineCallback_Intf
{
public:
	virtual void getValue(uint64_t key, size_t packet, uint8_t*& values, size_t& size) const = 0;
};

class AQENGINE_API AQEngine_Intf
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

AQENGINE_API AQEngine_Intf * getAQEngineSystem(aq::Base& base, aq::Settings& settings);
AQENGINE_API AQEngine_Intf * getAQEngineWindow(aq::Base& base, aq::Settings& settings);

}

#endif
