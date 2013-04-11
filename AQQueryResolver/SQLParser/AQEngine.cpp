#include "AQEngine.h"
#include <aq/Exceptions.h>
#include "JeqParser.h"
#include "SQLPrefix.h"
#include "TreeUtilities.h"
#include <string>
#include <direct.h>
#include <aq/Logger.h>
#include <aq/Timer.h>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace aq;
using namespace std;


AQEngine::AQEngine(Base& _baseDesc, TProjectSettings& settings)
	: baseDesc(_baseDesc)
{
	aqMatrix.reset(new aq::AQMatrix(settings));
}


AQEngine::~AQEngine(void)
{
}

void AQEngine::call(TProjectSettings& settings, tnode *pNode, int mode, int selectLevel)
{
	std::string query;
	syntax_tree_to_prefix_form( pNode, query );
	ParseJeq( query );
	
	if (query.size() < 2048) // fixme
		aq::Logger::getInstance().log(AQ_DEBUG, "\n%s\n", query.c_str());

#if defined(_DEBUG)
	std::cout << std::endl << query << std::endl << std::endl;
	std::string queryStr; 
	// syntax_tree_to_sql_form(pNode, queryStr);
	std::cout << std::endl << queryStr << std::endl << std::endl;
#endif

	//
	//
	SaveFile( settings.szOutputFN, query.c_str() );

	//
	// get temporary files created by previous calls
	DeleteFolder( settings.szTempPath1 );
	char path[_MAX_PATH];
	sprintf( path, "%s_%d", settings.szTempPath1, selectLevel );
	rename( path, settings.szTempPath1 );

	//
	// If mono table query, read PRM or TMP files to get the rows indexes
	//std::string tableName;
	//tnode * constraint = find_main_node(pNode, K_WHERE);
	//if (isMonoTable(pNode, tableName) && (constraint == NULL))
	//{
	//	this->generateAQMatrixFromPRM(tableName, constraint);
	//	return;
	//} 

	//
	// create folders for the engine
	mkdir( settings.szTempPath1 );
	mkdir( settings.szTempPath2 );
	vector<std::string> files;
	

	aq::Timer timer;
	if ((mode == 0) || (settings.executeNestedQuery))
	{
    int rc = 1;
    std::string prg = settings.szEnginePath;
    std::string arg = mode == 0 ? settings.szEngineParamsDisplay : settings.szEngineParamsNoDisplay;

    aq::Logger::getInstance().log(AQ_NOTICE, "call: '%s %s'\n", prg.c_str(), arg.c_str());

#if defined(WIN32) && defined(USE_CREATE_PROCESS)
    STARTUPINFOW si;
    // LPSTARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::wstring wprg = aq::string2Wstring(prg);
    std::wstring warg = aq::string2Wstring(arg);
    LPCWSTR prg_wstr = wprg.c_str();
    LPCWSTR arg_wstr = warg.c_str();
    //if (CreateProcess(prg.c_str(), (LPSTR)arg.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, si, &pi))
    if (CreateProcessW(prg_wstr, (LPWSTR)arg_wstr, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    //if (CreateProcess(NULL, "E:/Project_AQ/Bin/AQ_Engine.exe E:/AQ_DATABASES/DB/MSALGOQUEST/calculus/test_aq_engine/aqengine.ini test_aq_engine Dpy", 
    //  NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, si, &pi))
    {
      rc = WaitForSingleObject(pi.hProcess, INFINITE);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    }
#else
    rc = system((prg + " " + arg).c_str());
#endif
    
    if (rc != 0)
    {
			std::ostringstream oss;
			oss << "call to '" << prg << " " << arg << "' aq engine failed [ExitCode:" << rc << "]";
			aq::Logger::getInstance().log(AQ_ERROR, "%s\n", oss.str().c_str());
      if (query.size() < 2048) // fixme
        aq::Logger::getInstance().log(AQ_ERROR, "%s\n", query.c_str());
			throw generic_error(generic_error::AQ_ENGINE, oss.str());
		}
	}
	else
	{
		aq::Logger::getInstance().log(AQ_DEBUG, "do not call aq engine for nested query\n");
	}

	aq::Logger::getInstance().log(AQ_NOTICE, "AQ Engine Time elapsed: %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());

	switch( mode )
	{
	case 0:
		DeleteFolder( settings.szTempPath1 );
		break;
	case 1:
	{
		if( GetFiles( settings.szTempPath1, files ) != 0 )
			throw generic_error(generic_error::COULD_NOT_OPEN_FILE, "");
		--selectLevel;
		char path[_MAX_PATH];
		sprintf( path, "%s_%d", settings.szTempPath1, selectLevel );
		mkdir( path );
		for( size_t idx = 0; idx < files.size(); ++idx )
			//BxxxTxxxxTPNxxxxPxxxxxxxxxx.s
			if( files[idx].length() == 32
				&& files[idx][0] == 'B'
				&& files[idx][4] == 'T'
				&& files[idx][9] == 'T'
				&& files[idx][10] == 'P'
				&& files[idx][11] == 'N'
				&& files[idx][17] == 'P'
				&& files[idx][30] == '.'
				&& (files[idx][31] == 's'
				|| files[idx][31] == 't') )
			{
				char oldFile[_MAX_PATH];
				sprintf( oldFile, "%s\\%s", settings.szTempPath1, 
					files[idx].c_str() );
				
				char newFile[_MAX_PATH];
				std::string substr1 = files[idx].substr(0, 9).c_str();
				std::string substr2 = files[idx].substr(17, 14);
				sprintf( newFile, "%s\\%stmp", path, 
					substr1.c_str(), substr2.c_str() );
				
				remove( newFile );
				rename( oldFile, newFile );
			}
		DeleteFolder( settings.szTempPath1 );
	}
		break;
	default:
		throw generic_error(generic_error::NOT_IMPLEMENED, "");
	}

	timer.start();
	tableIDs.clear();
	aqMatrix->load(settings.szAnswerFN, settings.fieldSeparator, tableIDs);
	aq::Logger::getInstance().log(AQ_INFO, "Load From AQ Matrix: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
}

void AQEngine::generateAQMatrixFromPRM(const std::string tableName, tnode * whereNode)
{
	size_t tableIdx = this->baseDesc.getTableIdx(tableName);
	size_t nbRows = this->baseDesc.Tables[tableIdx].TotalCount;
	this->aqMatrix->simulate(nbRows, 2);
	this->tableIDs.clear();
	this->tableIDs.push_back(this->baseDesc.Tables[tableIdx].ID);
}