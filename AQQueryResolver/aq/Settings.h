#ifndef __AQ_SETTINGS_H__
#define __AQ_SETTINGS_H__

#if defined (WIN32)
# ifdef AQENGINE_EXPORTS
#  define AQENGINE_API __declspec(dllexport)
# else
#  define AQENGINE_API __declspec(dllimport)
# endif
#else
// # define AQENGINE_API __stdcall
# define AQENGINE_API
#endif

#include <string>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

#define STR_BUF_SIZE 4096

#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif

namespace aq
{

struct Settings 
{
  typedef boost::shared_ptr<Settings> Ptr;
  
  mutable std::stringstream output;

	std::string iniFile;
	std::string queryIdent;
	std::string outputFile;
	std::string answerFile;
	std::string dbDesc;
	std::string aqEngine;
	std::string aqLoader;
  std::string aqHome;
  std::string aqName;
  std::string rootPath;
  std::string workingPath;
	std::string tmpRootPath;
  std::string dataPath;
	std::string tmpPath;
	std::string dpyPath;

	char fieldSeparator;
	static const int MAX_COLUMN_NAME_SIZE = 255;
	
	size_t worker;
	size_t group_by_process_size;
  size_t process_thread;
	
  int packSize;
	int maxRecordSize;

	bool computeAnswer;
	bool csvFormat;
	bool skipNestedQuery;
  bool useBinAQMatrix;
  bool displayCount;
  bool cmdLine;
  bool trace;

	AQENGINE_API Settings();
	AQENGINE_API Settings(const Settings&);
	AQENGINE_API ~Settings();
	AQENGINE_API Settings& operator=(const Settings&);

  AQENGINE_API void initPath(const std::string& root);
  AQENGINE_API void load(const std::string& iniFile, const std::string& queryIdent);
	AQENGINE_API void load(const std::string& iniFile);
	AQENGINE_API void load(std::istream& is);
	AQENGINE_API void changeIdent(const std::string& queryIdent);
  AQENGINE_API void dump(std::ostream& os) const;
  AQENGINE_API std::string to_string() const;
  AQENGINE_API void writeAQEngineIni(std::ostream& os) const;
};

}

#endif
