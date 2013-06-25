#include <aq/Logger.h>
#include <boost/thread.hpp>
#include <boost/random.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

void process(int x)
{
  std::ostringstream id;
  id << boost::this_thread::get_id();
  aq::Logger::getInstance().log(AQ_INFO, "%s check every %d milliseconds\n", id.str().c_str(), x * 10);
  for (unsigned int i = 0; i < 10; i++)
  {
    aq::Logger::getInstance().log(AQ_CRITICAL, "%s\n", id.str().c_str());
    aq::Logger::getInstance().log(AQ_ERROR, "%s\n", id.str().c_str());
    aq::Logger::getInstance().log(AQ_WARNING, "%s\n", id.str().c_str());
    aq::Logger::getInstance().log(AQ_NOTICE, "%s\n", id.str().c_str());
    aq::Logger::getInstance().log(AQ_INFO, "%s\n", id.str().c_str());
    aq::Logger::getInstance().log(AQ_DEBUG, "%s\n", id.str().c_str());
    boost::this_thread::sleep(boost::posix_time::milliseconds(x * 10));
  }
}

int main(int argc, char ** argv)
{
  std::string mode;
  std::string ident;
  unsigned int level = AQ_LOG_DEBUG;
  bool lock_mode = false;
  bool date_mode = false;
  bool pid_mode = false;

  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("output", po::value<std::string>(&mode)->default_value("STDOUT"), "[STDOUT|LOCALFILE|SYSLOG]")
    ("level", po::value<unsigned int>(&level), "CRITICAL(2), ERROR(3), WARNING(4), NOTICE(5), INFO(6), DEBUG(7)")
    ("with-lock", po::bool_switch(&lock_mode), "for multithread program")
    ("with-date", po::bool_switch(&date_mode), "add date to log")
    ("with-pid", po::bool_switch(&pid_mode), "add thread id to log")
    ("ident", po::value<std::string>(&ident)->default_value("check-log.log"), "")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);    

  if (vm.count("help")) 
  {
    std::cout << desc << "\n";
    return 1;
  }

  aq::Logger::getInstance(ident.c_str(), mode == "STDOUT" ? STDOUT : mode == "LOCALFILE" ? LOCALFILE : mode == "SYSLOG" ? SYSLOG : STDOUT);
  aq::Logger::getInstance().setLevel(level);
  aq::Logger::getInstance().setLockMode(lock_mode);
  aq::Logger::getInstance().setDateMode(date_mode);
  aq::Logger::getInstance().setPidMode(pid_mode);

  boost::thread_group grp;
  boost::random::mt19937 rng;
  boost::random::uniform_int_distribution<> eight(1,8);
  for (unsigned int i = 0; i < 8; i++)
  {     
    grp.create_thread(boost::bind(process, eight(rng)));
  }
  grp.join_all();
  return EXIT_SUCCESS;
}
