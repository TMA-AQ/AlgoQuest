#ifndef __AQ_SETTINGS_H__
#define __AQ_SETTINGS_H__

#include <string>
#include <cstdint>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

#define STR_BUF_SIZE 4096

#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif

namespace aq
{

struct TProjectSettings {
  typedef boost::shared_ptr<TProjectSettings> Ptr;

	std::string iniFile;
	std::string output;
	std::string queryIdent;
  std::string szRootPath;
	std::string szEnginePath;
	
	std::string szTempRootPath;
	std::string szLoaderPath;
	char szSQLReqFN[ _MAX_PATH ];
	char szDBDescFN[ _MAX_PATH ];
	char szOutputFN[ _MAX_PATH ];
	char szAnswerFN[ _MAX_PATH ];
	char szThesaurusPath[ _MAX_PATH ];
	char szTempPath1[ _MAX_PATH ];
	char szTempPath2[ _MAX_PATH ];
	char szEngineParamsDisplay[ STR_BUF_SIZE ];
	char szEngineParamsNoDisplay[ STR_BUF_SIZE ];
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
	bool useRowResolver;
  bool useBinAQMatrix;

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
