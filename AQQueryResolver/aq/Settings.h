#ifndef __AQ_SETTINGS_H__
#define __AQ_SETTINGS_H__

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

#define STR_BUF_SIZE 4096

#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif

namespace aq
{

struct TProjectSettings 
{
  typedef boost::shared_ptr<TProjectSettings> Ptr;

	std::string iniFile;
	std::string queryIdent;
	std::string outputFile;
	std::string answerFile;
	std::string dbDesc;
	std::string aqEngine;
	std::string aqLoader;
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
	bool executeNestedQuery;
  bool useBinAQMatrix;
  bool displayCount;

	TProjectSettings();
	TProjectSettings(const TProjectSettings&);
	~TProjectSettings();
	TProjectSettings& operator=(const TProjectSettings&);

  void load(const std::string& iniFile, const std::string& queryIdent);
	void load(const std::string& iniFile);
	void changeIdent(const std::string& queryIdent);
  void dump(std::ostream& os) const;
  void writeAQEngineIni(std::ostream& os) const;
};

}

#endif
