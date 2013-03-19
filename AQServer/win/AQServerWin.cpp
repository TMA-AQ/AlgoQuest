#include "aq/Link.h" // tma FIXME: just for link issue with SQL2Prefix library

#include <stdio.h> 
#include <windows.h> 
#include "ServiceInstaller.h" 
#include "ServiceBase.h" 
#include "AQService.h" 

extern int yylineno;
int yyerror( const char *pszMsg ) 
{
	std::cerr << "SQL Parsing Error : " << pszMsg << " encountered at line number: " << yylineno << std::endl;
	return 0;
}

////////////////////////////////
// Internal name of the service 
#define SERVICE_NAME             L"AQEngineService" 
#define SERVICE_DISPLAY_NAME     L"AlgoQuest Enterperprise Service" 
#define SERVICE_START_TYPE       SERVICE_DEMAND_START 
#define SERVICE_DEPENDENCIES     L"" 
#define SERVICE_ACCOUNT          L"NT AUTHORITY\\LocalService" 
#define SERVICE_PASSWORD         NULL 

int wmain(int argc, wchar_t *argv[]) 
{ 
	if ((argc > 1) && ((*argv[1] == L'-' || (*argv[1] == L'/')))) 
	{ 
		if (_wcsicmp(L"install", argv[1] + 1) == 0) 
		{ 
			// Install the service when the command is  
			// "-install" or "/install". 
			InstallService( 
				SERVICE_NAME,               // Name of service 
				SERVICE_DISPLAY_NAME,       // Name to display 
				SERVICE_START_TYPE,         // Service start type 
				SERVICE_DEPENDENCIES,       // Dependencies 
				SERVICE_ACCOUNT,            // Service running account 
				SERVICE_PASSWORD            // Password of the account 
				); 
		} 
		else if (_wcsicmp(L"remove", argv[1] + 1) == 0) 
		{ 
			// Uninstall the service when the command is  
			// "-remove" or "/remove". 
			UninstallService(SERVICE_NAME); 
		} 
	} 
	else 
	{ 
		wprintf(L"Parameters:\n"); 
		wprintf(L" -install  to install the service.\n"); 
		wprintf(L" -remove   to remove the service.\n"); 


		AQService service(SERVICE_NAME); 
		if (!CServiceBase::Run(service)) 
		{ 
			wprintf(L"Service failed to run w/err 0x%08lx\n", GetLastError()); 
		} 
	} 


	return 0; 
} 

//namespace po = boost::program_options;
//
//int main(int argc, char ** argv)
//{
//  // log options
//  std::string mode;
//  unsigned int level = LOG_DEBUG;
//  bool lock_mode = false;
//  bool date_mode = false;
//  bool pid_mode = false;
//
//  // aq options
//  boost::uint16_t port;
//  std::string cfgFile;
//
//  po::options_description desc("Allowed options");
//  desc.add_options()
//    ("help", "produce help message")
//    ("log-output", po::value<std::string>(&mode)->default_value("STDOUT"), "")
//    ("log-level", po::value<unsigned int>(&level), "")
//    ("log-lock-mode", po::bool_switch(&lock_mode), "")
//    ("log-date-mode", po::bool_switch(&date_mode), "")
//    ("log-pid-mode", po::bool_switch(&pid_mode), "")
//    ("aq-port", po::value<boost::uint16_t>(&port)->default_value(9999), "")
//    ("aq-cfg", po::value<std::string>(&cfgFile)->default_value("config.xml"), "")
//    ;
//
//  po::variables_map vm;
//  po::store(po::parse_command_line(argc, argv, desc), vm);
//  po::notify(vm);    
//
//  if (vm.count("help")) 
//  {
//    std::cout << desc << "\n";
//    return 1;
//  }
//
//  aq::Logger::getInstance("check-log.log", mode == "STDOUT" ? STDOUT : mode == "LOCALFILE" ? LOCALFILE : mode == "SYSLOG" ? SYSLOG : STDOUT);
//  aq::Logger::getInstance().setLevel(level);
//  aq::Logger::getInstance().setLockMode(lock_mode);
//  aq::Logger::getInstance().setDateMode(date_mode);
//  aq::Logger::getInstance().setPidMode(pid_mode);
//
//  try
//  {
//    //
//    //
//    boost::shared_ptr<aq::Configuration> cfg(new aq::Configuration(cfgFile.c_str()));
//    cfg->dump(std::cerr);
//
//    aq::Logger::getInstance().log(AQ_NOTICE, "starting algoquest-server on %u\n", port);
//    boost::asio::io_service io_service;
//    aq::Server server(io_service, port, cfg);
//
//    io_service.run();
//  }
//  catch (const generic_error& ex)
//  {
//    aq::Logger::getInstance().log(AQ_ERROR, ex.what());
//    return EXIT_FAILURE;
//  }
//  catch (const std::exception& ex)
//  {
//    aq::Logger::getInstance().log(AQ_ERROR, ex.what());
//    return EXIT_FAILURE;
//  }
//
//  return EXIT_SUCCESS;
//}

