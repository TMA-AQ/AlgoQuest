#include "Settings.h"
#include <aq/Utilities.h>
#include <aq/Exceptions.h>
#include <fstream>
#include <aq/Logger.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>

TProjectSettings::TProjectSettings()
  : 
	iniFile(""),
	output(""),
	szRootPath(""),
	szEnginePath(""),
  szTempRootPath(""),
  szCutInColPath(""),
  szLoaderPath(""),
	worker(1),
	group_by_process_size(100000),
  packSize(1048576), 
  maxRecordSize(40960),
  computeAnswer(true),
	csvFormat(false),
	executeNestedQuery(true),
	useRowResolver(false)
{
  ::memset(szSQLReqFN, 0, _MAX_PATH);
  ::memset(szDBDescFN, 0, _MAX_PATH);
  ::memset(szOutputFN, 0, _MAX_PATH);
  ::memset(szAnswerFN, 0, _MAX_PATH);
  ::memset(szThesaurusPath, 0, _MAX_PATH);
  ::memset(szTempPath1, 0, _MAX_PATH);
  ::memset(szTempPath2, 0, _MAX_PATH);
  ::memset(szEngineParamsDisplay, 0, STR_BUF_SIZE);
  ::memset(szEngineParamsNoDisplay, 0, STR_BUF_SIZE);
}

TProjectSettings::TProjectSettings(const TProjectSettings& obj)
	:
	iniFile(obj.iniFile),
	output(obj.output),
	szRootPath(obj.szRootPath),
	szEnginePath(obj.szEnginePath),
  szTempRootPath(obj.szTempRootPath),
  szCutInColPath(obj.szCutInColPath),
  szLoaderPath(obj.szLoaderPath),
	fieldSeparator(obj.fieldSeparator),
	worker(obj.worker),
	group_by_process_size(obj.group_by_process_size),
	packSize(obj.packSize),
	maxRecordSize(obj.maxRecordSize),
	computeAnswer(obj.computeAnswer),
	executeNestedQuery(obj.executeNestedQuery),
	useRowResolver(obj.useRowResolver)
{
  ::strcpy(szSQLReqFN, obj.szSQLReqFN);
  ::strcpy(szDBDescFN, obj.szDBDescFN);
  ::strcpy(szOutputFN, obj.szOutputFN);
  ::strcpy(szAnswerFN, obj.szAnswerFN);
  ::strcpy(szThesaurusPath, obj.szThesaurusPath);
  ::strcpy(szTempPath1, obj.szTempPath1);
  ::strcpy(szTempPath2, obj.szTempPath2);
  ::strcpy(szEngineParamsDisplay, obj.szEngineParamsDisplay);
  ::strcpy(szEngineParamsNoDisplay, obj.szEngineParamsNoDisplay);
}

TProjectSettings::~TProjectSettings()
{
}

TProjectSettings& TProjectSettings::operator=(const TProjectSettings& obj)
{
	if (this != &obj)
	{
		fieldSeparator = obj.fieldSeparator;
		packSize = obj.packSize;
		maxRecordSize = obj.maxRecordSize;
		computeAnswer = obj.computeAnswer;
		iniFile = obj.iniFile;
		output = obj.output;
		szRootPath = obj.szRootPath;
		szTempRootPath = obj.szTempRootPath;
		szCutInColPath = obj.szCutInColPath;
		szLoaderPath = obj.szLoaderPath;
		szSQLReqFN, obj.szSQLReqFN;
		::strcpy(szDBDescFN, obj.szDBDescFN);
		::strcpy(szOutputFN, obj.szOutputFN);
		::strcpy(szAnswerFN, obj.szAnswerFN);
		::strcpy(szThesaurusPath, obj.szThesaurusPath);
		szEnginePath = obj.szEnginePath;
		fieldSeparator = obj.fieldSeparator;
		::strcpy(szTempPath1, obj.szTempPath1);
		::strcpy(szTempPath2, obj.szTempPath2);
		::strcpy(szEngineParamsDisplay, obj.szEngineParamsDisplay);
		::strcpy(szEngineParamsNoDisplay, obj.szEngineParamsNoDisplay);
		worker = obj.worker;
		group_by_process_size = obj.group_by_process_size;
		packSize = obj.packSize;
		maxRecordSize = obj.maxRecordSize;
		computeAnswer = obj.computeAnswer;
		executeNestedQuery = obj.executeNestedQuery;
		useRowResolver = obj.useRowResolver;
	}
	return *this;
}

void TProjectSettings::load(const std::string& iniFile, const std::string& queryIdent)
{
	this->load(iniFile);
	this->changeIdent(queryIdent);
}

void TProjectSettings::load(const std::string& iniFile)
{
	this->iniFile = iniFile;
  try
  {
    std::ifstream fin(iniFile.c_str(), std::ifstream::in);
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(fin, pt);
		
    this->szRootPath = pt.get<std::string>(boost::property_tree::ptree::path_type("root-folder"));
    this->szTempRootPath = pt.get<std::string>(boost::property_tree::ptree::path_type("tmp-folder"));
    this->fieldSeparator = pt.get<std::string>(boost::property_tree::ptree::path_type("field-separator")).at(0);
		
    this->szEnginePath = pt.get<std::string>(boost::property_tree::ptree::path_type("aq-engine"));
    this->szCutInColPath = pt.get<std::string>(boost::property_tree::ptree::path_type("cut-in-col"));
    this->szLoaderPath = pt.get<std::string>(boost::property_tree::ptree::path_type("loader"));
		
		// optional
		boost::optional<size_t> opt;
    opt = pt.get_optional<size_t>(boost::property_tree::ptree::path_type("worker"));
		if (opt.is_initialized()) this->worker = opt.get();
    opt = pt.get_optional<size_t>(boost::property_tree::ptree::path_type("group-by-process-size"));
		if (opt.is_initialized()) this->group_by_process_size = opt.get();

    //
    // Change '\' by '/'
    boost::algorithm::replace_all(this->szEnginePath, "\\", "/");
    boost::algorithm::trim(this->szEnginePath);
    boost::algorithm::replace_all(this->szCutInColPath, "\\", "/");
    boost::algorithm::trim(this->szCutInColPath);
    boost::algorithm::replace_all(this->szLoaderPath, "\\", "/");
    boost::algorithm::trim(this->szLoaderPath);

    boost::algorithm::replace_all(this->szRootPath, "\\", "/");
    boost::algorithm::trim(this->szRootPath);
    boost::algorithm::replace_all(this->szTempRootPath, "\\", "/");
    boost::algorithm::trim(this->szTempRootPath);
		
		//
		// add '/' at end of directory if needed
		if (*this->szRootPath.rbegin() != '/') this->szRootPath += "/";
		if (*this->szTempRootPath.rbegin() != '/') this->szTempRootPath += "/";

		//
		// base desc file
		strcpy( this->szDBDescFN, this->szRootPath.c_str() );
		strcat( this->szDBDescFN, "base_struct/base." );
		
		//
		// thesaurus path
		strcpy( this->szThesaurusPath, this->szRootPath.c_str() );
		strcat( this->szThesaurusPath, "data_orga/vdg/data/" );
    
	}
	catch (const boost::property_tree::ptree_error& e)
	{
    std::ostringstream oss;
    oss << "invalid properties file: " << iniFile << " [" << e.what() << "]" << std::endl;
    throw aq::generic_error(aq::generic_error::INVALID_FILE, oss.str());
	}
}

void TProjectSettings::changeIdent(const std::string& _queryIdent)
{
	this->queryIdent = _queryIdent;
	
	//
	// request file path (deprecated)
	strcpy( this->szSQLReqFN, this->szRootPath.c_str() );
	strcat( this->szSQLReqFN, "/calculus/" );
	strcat( this->szSQLReqFN, queryIdent.c_str() );
	strcat( this->szSQLReqFN, "/Request.txt" );

	//
	// new request path
	strcpy( this->szOutputFN, this->szRootPath.c_str() );
	strcat( this->szOutputFN, "/calculus/" );
	strcat( this->szOutputFN, queryIdent.c_str() );
	strcat( this->szOutputFN, "/New_Request.txt" );

	//
	// answer path (deprecated)
	strcpy( this->szAnswerFN, this->szRootPath.c_str() );
	strcat( this->szAnswerFN, "/calculus/" );
	strcat( this->szAnswerFN, queryIdent.c_str() );
	strcat( this->szAnswerFN, "/Answer.txt" );

	//
	// tempory path
	strcpy( this->szTempPath1, this->szTempRootPath.c_str() );
	strcat( this->szTempPath1, "/" );
	strcat( this->szTempPath1, queryIdent.c_str() );
	strcpy( this->szTempPath2, this->szTempPath1 );
	strcat( this->szTempPath2, "/dpy" );
	
	//
	// change ini file
	iniFile = this->szRootPath + "/calculus/" + queryIdent + "/aqengine.ini";

	//
	// Prepare engine arguments
	sprintf( this->szEngineParamsDisplay, "%s %s Dpy", iniFile.c_str(), queryIdent.c_str() );
	sprintf( this->szEngineParamsNoDisplay, "%s %s NoDpy", iniFile.c_str(), queryIdent.c_str() );

}

void TProjectSettings::dump(std::ostream& os) const
{
  os << "szRootPath: '" << szRootPath << "'" << std::endl;
	os << "szTempRootPath: '" << szTempRootPath << "'" << std::endl;
	os << "szCutInColPath: '" << szCutInColPath << "'" << std::endl;
	os << "szLoaderPath: '" << szLoaderPath << "'" << std::endl;
	os << "szSQLReqFN: '" << szSQLReqFN << "'" << std::endl;
	os << "szDBDescFN: '" << szDBDescFN << "'" << std::endl;
	os << "szOutputFN: '" << szOutputFN << "'" << std::endl;
	os << "szAnswerFN: '" << szAnswerFN << "'" << std::endl;
	os << "szThesaurusPath: '" << szThesaurusPath << "'" << std::endl;
	os << "szEnginePath: '" << szEnginePath << "'" << std::endl;
	os << "szTempPath1: '" << szTempPath1 << "'" << std::endl;
	os << "szTempPath2: '" << szTempPath2 << "'" << std::endl;
	os << "szEngineParamsDisplay: '" << szEngineParamsDisplay << "'" << std::endl;
	os << "szEngineParamsNoDisplay: '" << szEngineParamsNoDisplay << "'" << std::endl;
	os << "fieldSeparator: '" << fieldSeparator << "'" << std::endl;
	os << "MAX_COLUMN_NAME_SIZE: '" << MAX_COLUMN_NAME_SIZE << "'" << std::endl;
	os << "packSize: '" << packSize << "'" << std::endl;
	os << "maxRecordSize: '" << maxRecordSize << "'" << std::endl;
  os << "computeAnswer: '" << computeAnswer << "'" << std::endl;
  os << "useRowResolver: '" << useRowResolver << "'" << std::endl;
}

void TProjectSettings::writeAQEngineIni(std::ostream& os) const
{
	os << "export.filename.final=" << szDBDescFN << std::endl;
	os << "step1.field.separator=" << fieldSeparator << std::endl;
  os << "k_rep_racine=" << szRootPath << std::endl;
	os << "k_rep_racine_tmp=" << szRootPath << std::endl;
}
