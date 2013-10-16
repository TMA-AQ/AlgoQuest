// AQPythonBinding.cpp : définit les fonctions exportées pour l'application DLL.
//

#include "../aq/Util.h"
#include <aq/Logger.h>
#include <aq/Timer.h>
#include <aq/Exceptions.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

namespace py = boost::python;

template<class T>
py::list std_vector_to_py_list(const std::vector<T>& v)
{
    py::object get_iter = py::iterator<std::vector<T> >();
    py::object iter = get_iter(v);
    py::list l(iter);
    return l;
}

namespace aq
{

typedef std::vector<std::string> row_t;
typedef std::vector<row_t> result_t;

void set_log_level(unsigned int level)
{
  aq::Logger::getInstance().setLevel(level);
}

struct cb_rows : public display_cb
{
  cb_rows() 
  {
    result.push_back(std::vector<std::string>());
  }
  void push(const std::string& value)
  {
    (*result.rbegin()).push_back(value);
  }
  void next()
  {
    result.push_back(std::vector<std::string>());
  }
  result_t result;
};

const result_t execute(aq::opt cfg, const std::string& query)
{
  int rc = 0;
  cb_rows * cb = new cb_rows;

  try
  {
    if (cfg.workingPath == "")
      cfg.workingPath = cfg.dbPath;

    std::vector<std::string> selectedColumns;
    aq::get_columns(selectedColumns, query, "SELECT");

    std::string iniFilename;
    cfg.force = true;
    aq::generate_working_directories(cfg, iniFilename);

    std::ofstream queryFile(std::string(cfg.dbPath + "calculus/" + cfg.queryIdent + "/New_Request.txt").c_str());
    queryFile << query ;
    queryFile.close();

    aq::Timer timer;
    rc = aq::run_aq_engine(cfg.aqEngine, iniFilename, cfg.queryIdent);
    std::string aq_engine_time_elapsed = aq::Timer::getString(timer.getTimeElapsed());
    // std::cout << "aq engine performed in " << aq_engine_time_elapsed << std::endl;

    std::stringstream ss;
    std::string matrix_path = cfg.workingPath + "/data_orga/tmp/" + cfg.queryIdent + "/dpy/";
    cfg.packetSize = aq::packet_size; // FIXME
    rc = aq::display(cb, matrix_path, cfg, selectedColumns);

    boost::filesystem::path tmpPath(cfg.workingPath + "/data_orga/tmp/" + cfg.queryIdent + "/");
    boost::filesystem::remove_all(tmpPath);
  }
  catch (const aq::generic_error& ex)
  {
    aq::Logger::getInstance().log(AQ_ERROR, ex.what());
  }

  return cb->result;
}

const char * version()
{
  return "0.0.1";
}

}

using namespace boost::python;

BOOST_PYTHON_MODULE(AlgoQuestDB)
{
  class_<aq::row_t>("AQRow").def(vector_indexing_suite<aq::row_t>());
  class_<aq::result_t>("AQResult").def(vector_indexing_suite<aq::result_t>());

  class_<aq::opt>("Settings", init<>())
    .def_readwrite("dbPath", &aq::opt::dbPath)
    .def_readwrite("workingPath", &aq::opt::workingPath)
    .def_readwrite("engine", &aq::opt::aqEngine)
    .def_readwrite("ident", &aq::opt::queryIdent)
    .def_readwrite("count", &aq::opt::withCount)
    .def_readwrite("index", &aq::opt::withIndex)
    ;

  def("Version", aq::version);
  def("SetLogLevel", aq::set_log_level);
  def("Execute", aq::execute);

}
