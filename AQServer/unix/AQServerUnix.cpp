#include <SQLParser/AQEngine.h>
#include <SQLParser/SQLParser.h>
#include <SQLParser/SQLPrefix.h>
#include <SQLParser/Column2Table.h>
#include <SQLParser/NestedQueries.h>
#include <SQLParser/JeqParser.h>
#include <SQLParser/Exceptions.h>

#include <aq/sync/SyncServer.h>
#include <aq/async/AsyncServer.h>
#include <aq/Configuration.h>
#include <cstdlib>
#include <iostream>
#include <boost/asio/io_service.hpp>
#include <boost/program_options.hpp>
#include <aq/Logger.h>

#include "aq/Link.h" // tma FIXME: just for link issue with SQL2Prefix library

extern int yylineno;
int yyerror( const char *pszMsg ) 
{
	std::cerr << "SQL Parsing Error : " << pszMsg << " encountered at line number: " << yylineno << std::endl;
	return 0;
}

namespace po = boost::program_options;

int main(int argc, char ** argv)
{
	// log options
	std::string mode;
	unsigned int level;
	unsigned int ioWorker;
	bool lock_mode = false;
	bool date_mode = false;
	bool pid_mode = false;

	// aq options
	boost::uint16_t port;
	std::string cfgFile;
	bool useAsync = false;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("log-output", po::value<std::string>(&mode)->default_value("STDOUT"), "")
		("log-level", po::value<unsigned int>(&level)->default_value(LOG_INFO), "")
		("log-lock", po::bool_switch(&lock_mode), "")
		("log-date", po::bool_switch(&date_mode), "")
		("log-pid", po::bool_switch(&pid_mode), "")
		("aq-port", po::value<boost::uint16_t>(&port)->default_value(9999), "")
		("aq-cfg", po::value<std::string>(&cfgFile)->default_value("config.xml"), "")
		("io-worker", po::value<unsigned int>(&ioWorker)->default_value(4), "")
		("use-async", po::bool_switch(&useAsync), "")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);    

	if (vm.count("help")) 
	{
		std::cout << desc << "\n";
		return 1;
	}

	aq::Logger::getInstance("check-log.log", mode == "STDOUT" ? STDOUT : mode == "LOCALFILE" ? LOCALFILE : mode == "SYSLOG" ? SYSLOG : STDOUT);
	aq::Logger::getInstance().setLevel(level);
	aq::Logger::getInstance().setLockMode(lock_mode);
	aq::Logger::getInstance().setDateMode(date_mode);
	aq::Logger::getInstance().setPidMode(pid_mode);

	try
	{
		//
		//
		boost::shared_ptr<aq::Configuration> cfg(new aq::Configuration(cfgFile.c_str()));
		cfg->dump(std::cerr);

		aq::Logger::getInstance().log(AQ_NOTICE, "starting algoquest-server on %u\n", port);
		boost::asio::io_service io_service;

		if (useAsync)
			boost::shared_ptr<aq::async::Server> server(new aq::async::Server(io_service, port, cfg));
		else 
			boost::shared_ptr<aq::sync::Server> server(new aq::sync::Server(io_service, port, cfg));

		boost::thread_group grp;
		for (unsigned int i = 0; i < ioWorker; i++)
		{
			grp.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
		}
		grp.join_all();
	}
	catch (const generic_error& ex)
	{
		aq::Logger::getInstance().log(AQ_ERROR, ex.what());
		return EXIT_FAILURE;
	}
	catch (const std::exception& ex)
	{
		aq::Logger::getInstance().log(AQ_ERROR, ex.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

