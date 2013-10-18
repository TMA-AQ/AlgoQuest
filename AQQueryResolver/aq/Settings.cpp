#include "Settings.h"
#include <aq/Utilities.h>
#include <aq/Exceptions.h>
#include <aq/BaseDesc.h>
#include <fstream>
#include <aq/Logger.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>

namespace aq
{

TProjectSettings::TProjectSettings()
  : 
	iniFile(""),
  queryIdent(""),
	outputFile(""),
	answerFile(""),
	dbDesc(""),
	aqEngine(""),
  aqLoader(""),
	rootPath(""),
  workingPath(""),
  tmpRootPath(""),
  dataPath(""),
	tmpPath(""),
	dpyPath(""),
	worker(1),
	group_by_process_size(100000),
  process_thread(1),
  packSize(aq::packet_size), 
  maxRecordSize(40960),
  computeAnswer(true),
	csvFormat(false),
	executeNestedQuery(true),
  useBinAQMatrix(true),
  displayCount(false)
{
}

TProjectSettings::TProjectSettings(const TProjectSettings& obj)
	:
	iniFile(obj.iniFile),
	outputFile(obj.outputFile),
	answerFile(obj.answerFile),
  queryIdent(obj.queryIdent),
	dbDesc(obj.dbDesc),
	aqEngine(obj.aqEngine),
  aqLoader(obj.aqLoader),
	rootPath(obj.rootPath),
  workingPath(obj.workingPath),
  tmpRootPath(obj.tmpRootPath),
  dataPath(obj.dataPath),
	tmpPath(obj.tmpPath),
	dpyPath(obj.dpyPath),
	fieldSeparator(obj.fieldSeparator),
	worker(obj.worker),
	group_by_process_size(obj.group_by_process_size),
  process_thread(obj.process_thread),
	packSize(obj.packSize),
	maxRecordSize(obj.maxRecordSize),
	computeAnswer(obj.computeAnswer),
	executeNestedQuery(obj.executeNestedQuery),
  useBinAQMatrix(obj.useBinAQMatrix),
  displayCount(obj.displayCount)
{
}

TProjectSettings::~TProjectSettings()
{
}

TProjectSettings& TProjectSettings::operator=(const TProjectSettings& obj)
{
	if (this != &obj)
	{
		iniFile = obj.iniFile;
    queryIdent = obj.queryIdent;
		outputFile = obj.outputFile;
    answerFile = obj.answerFile;
    dbDesc = obj.dbDesc;
		aqEngine = obj.aqEngine;
    aqLoader = obj.aqLoader;
		rootPath = obj.rootPath;
    workingPath = obj.workingPath;
		tmpRootPath = obj.tmpRootPath;
    dataPath = obj.dataPath;
    tmpPath = obj.tmpPath;
    dpyPath = obj.dpyPath;
    fieldSeparator = obj.fieldSeparator;
		worker = obj.worker;
		group_by_process_size = obj.group_by_process_size;
    process_thread = obj.process_thread;
		packSize = obj.packSize;
		maxRecordSize = obj.maxRecordSize;
		computeAnswer = obj.computeAnswer;
		maxRecordSize = obj.maxRecordSize;
		computeAnswer = obj.computeAnswer;
		executeNestedQuery = obj.executeNestedQuery;
    useBinAQMatrix = obj.useBinAQMatrix;
    displayCount = obj.displayCount;
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
		
    this->rootPath = pt.get<std::string>(boost::property_tree::ptree::path_type("root-folder"));
    this->tmpRootPath = pt.get<std::string>(boost::property_tree::ptree::path_type("tmp-folder"));
    this->fieldSeparator = pt.get<std::string>(boost::property_tree::ptree::path_type("field-separator")).at(0);
		
    this->aqEngine = pt.get<std::string>(boost::property_tree::ptree::path_type("aq-engine"));
    this->aqLoader = pt.get<std::string>(boost::property_tree::ptree::path_type("aq-loader"));
		
		// optional
		boost::optional<size_t> opt;
    opt = pt.get_optional<size_t>(boost::property_tree::ptree::path_type("worker"));
		if (opt.is_initialized()) this->worker = opt.get();
    opt = pt.get_optional<size_t>(boost::property_tree::ptree::path_type("group-by-process-size"));
		if (opt.is_initialized()) this->group_by_process_size = opt.get();
    opt = pt.get_optional<size_t>(boost::property_tree::ptree::path_type("process-thread"));
    if (opt.is_initialized()) this->process_thread = opt.get();
    boost::optional<bool> o = pt.get_optional<bool>(boost::property_tree::ptree::basic_ptree::path_type("display-count"));
    if (o.is_initialized()) this->displayCount = o.get();

    //
    // Change '\' by '/'
    boost::algorithm::replace_all(this->aqEngine, "\\", "/");
    boost::algorithm::trim(this->aqEngine);
    boost::algorithm::replace_all(this->aqLoader, "\\", "/");
    boost::algorithm::trim(this->aqLoader);

    boost::algorithm::replace_all(this->rootPath, "\\", "/");
    boost::algorithm::trim(this->rootPath);
    boost::algorithm::replace_all(this->tmpRootPath, "\\", "/");
    boost::algorithm::trim(this->tmpRootPath);
		
		//
		// add '/' at end of directory if needed
		if (*this->rootPath.rbegin() != '/') this->rootPath += "/";
		if (*this->tmpRootPath.rbegin() != '/') this->tmpRootPath += "/";

		//
		// base desc file
		this->dbDesc = this->rootPath + "base_struct/base.xml";
    boost::filesystem::path bdf(this->dbDesc);
    if (!boost::filesystem::exists(bdf))
    {
      this->dbDesc = this->rootPath + "base_struct/base.aqb";
    }

		//
		// data path
    this->dataPath = this->rootPath + "data_orga/vdg/data/";
    
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
	
  this->workingPath = this->rootPath + "calculus/" + queryIdent + "/";
  this->answerFile = this->workingPath + "Answer.txt";

	//
	// tempory path
  this->tmpPath = this->tmpRootPath + queryIdent + "/";
  this->dpyPath = this->tmpPath + "dpy/";
	
	//
	// change ini file
	this->iniFile = this->rootPath + "/calculus/" + queryIdent + "/aqengine.ini";
}

void TProjectSettings::dump(std::ostream& os) const
{
  os << "root-path:            ['" << rootPath             << "']" << std::endl;
  os << "working-path:         ['" << workingPath          << "']" << std::endl;
	os << "tmp-root-path:        ['" << tmpRootPath          << "']" << std::endl;
	os << "dataPath:             ['" << dataPath             << "']" << std::endl;
	os << "tmpPath:              ['" << tmpPath              << "']" << std::endl;
	os << "dpyPath:              ['" << dpyPath              << "']" << std::endl;
	os << "db-desc:              ['" << dbDesc               << "']" << std::endl;
	os << "aq-engine:            ['" << aqEngine             << "']" << std::endl;
	os << "aq-loader:            ['" << aqLoader             << "']" << std::endl;
	os << "output:               ['" << outputFile           << "']" << std::endl;
	os << "answer:               ['" << answerFile           << "']" << std::endl;
	os << "fieldSeparator:       ['" << fieldSeparator       << "']" << std::endl;
	os << "MAX_COLUMN_NAME_SIZE: ["  << MAX_COLUMN_NAME_SIZE <<  "]" << std::endl;
	os << "packSize:             ["  << packSize             <<  "]" << std::endl;
	os << "maxRecordSize:        ["  << maxRecordSize        <<  "]" << std::endl;
  os << "computeAnswer:        ["  << computeAnswer        <<  "]" << std::endl;
  os << "displayCount:         ["  << displayCount         <<  "]" << std::endl;
}

void TProjectSettings::writeAQEngineIni(std::ostream& os) const
{
	os << "export.filename.final=" << dbDesc << std::endl;
	os << "step1.field.separator=" << fieldSeparator << std::endl;
  os << "k_rep_racine=" << rootPath << std::endl;
  // FIXME
  std::string::size_type pos = tmpRootPath.find("data_orga/tmp/");
  if (pos != std::string::npos)
  {
    os << "k_rep_racine_tmp=" << tmpRootPath.substr(0, pos) << std::endl;
  }
  else
  {
    os << "k_rep_racine_tmp=" << tmpRootPath << std::endl;
  }
}

}
