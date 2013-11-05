#ifndef __AQ_SETTINGS_H__
#define __AQ_SETTINGS_H__

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

	Settings();
	Settings(const Settings&);
	~Settings();
	Settings& operator=(const Settings&);

  void initPath(const std::string& root);
  void load(const std::string& iniFile, const std::string& queryIdent);
	void load(const std::string& iniFile);
	void load(std::istream& is);
	void changeIdent(const std::string& queryIdent);
  void dump(std::ostream& os) const;
  std::string to_string() const;
  void writeAQEngineIni(std::ostream& os) const;
};

}

#endif
