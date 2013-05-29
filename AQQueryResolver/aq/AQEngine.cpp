#include "AQEngine.h"
#include <aq/Exceptions.h>
#include "parser/JeqParser.h"
#include "SQLPrefix.h"
#include "TreeUtilities.h"
#include <string>
#include <direct.h>
#include <aq/Logger.h>
#include <aq/Timer.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace aq
{

AQEngine::AQEngine(Base& _baseDesc, TProjectSettings& _settings)
	: baseDesc(_baseDesc), settings(_settings)
{
}


AQEngine::~AQEngine(void)
{
}

void AQEngine::call(aq::tnode *pNode, mode_t mode, int selectLevel)
{
	std::string query;
  aq::syntax_tree_to_prefix_form( pNode, query );

  // FIXME : change ORDER by GROUP => this is temporary
  std::string::size_type pos = query.find("ORDER");
  if (pos != std::string::npos)
  {
    query.replace(pos, 5, "GROUP");
  }

  // FIXME
  pos = query.find("GROUP");
  if (pos != std::string::npos)
  {
    std::string queryTmp = query.substr(0, pos);
    ParseJeq( queryTmp );
    if (query.substr(pos).size() > 5) // Group By can be empty
      query = queryTmp + query.substr(pos);
  }
  else
  {
    ParseJeq( query );
  }
	
	if (query.size() < 2048) // fixme
		aq::Logger::getInstance().log(AQ_DEBUG, "\n%s\n", query.c_str());

#if defined(_DEBUG)
	std::cout << std::endl << query.substr(0, 1024) << std::endl << std::endl;
	// std::string queryStr; 
	// syntax_tree_to_sql_form(pNode, queryStr);
	// std::cout << std::endl << queryStr.substr(0, 1024) << std::endl << std::endl;
#endif

	//
	//
	aq::SaveFile( settings.szOutputFN, query.c_str() );

	//
	// get temporary files created by previous calls
	//aq::DeleteFolder( settings.szTempPath1 );
	//char path[_MAX_PATH];
	//sprintf( path, "%s_%d", settings.szTempPath1, selectLevel );
	//rename( path, settings.szTempPath1 );

	//
	// If mono table query, read PRM or TMP files to get the rows indexes
	//std::string tableName;
	//aq::tnode * constraint = find_main_node(pNode, K_WHERE);
	//if (isMonoTable(pNode, tableName) && (constraint == NULL))
	//{
	//	this->generateAQMatrixFromPRM(tableName, constraint);
	//	return;
	//} 

	//
	// create folders for the engine
	// mkdir( settings.szTempPath1 );
	mkdir( settings.szTempPath2 );

	aq::Timer timer;
	if ((mode == 0) || (settings.executeNestedQuery))
	{
    int rc = 1;
    std::string prg = settings.szEnginePath;
    std::string arg = mode == NESTED_2 ? settings.szEngineParamsNoDisplay : settings.szEngineParamsDisplay;
    if ((rc = this->run(prg.c_str(), arg.c_str())) != 0)
    {
			aq::Logger::getInstance().log(AQ_ERROR, "call to %s %s failed [ExitCode:%d]\n", prg.c_str(), arg.c_str(), rc);
      if (query.size() < 2048) // fixme
        aq::Logger::getInstance().log(AQ_ERROR, "\n\nAQEngine ERROR:\n%s\n", query.c_str());
			throw aq::generic_error(aq::generic_error::AQ_ENGINE, "call to %s %s failed [ExitCode:%d]", prg.c_str(), arg.c_str(), rc);
		}
	}
	else
	{
		aq::Logger::getInstance().log(AQ_DEBUG, "do not call aq engine for nested query\n");
	}

	aq::Logger::getInstance().log(AQ_NOTICE, "AQ Engine Time elapsed: %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());

	if ((mode == REGULAR) || (mode == NESTED_1))
	{
    if (mode == REGULAR)
    {
      aq::DeleteFolder( settings.szTempPath2 );
    }
    else
    {
      aq::CleanFolder( settings.szTempPath1 );
    }
    timer.start();
    tableIDs.clear();
    aqMatrix.reset(new aq::AQMatrix(settings));
    
    //aqMatrix->clear();
    if (settings.useBinAQMatrix)
    {
      aqMatrix->load(settings.szAnswerFN, this->tableIDs);
      aq::Logger::getInstance().log(AQ_NOTICE, "Load From Binary AQ Matrix: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
    }
    else
    {
      aqMatrix->load(settings.szAnswerFN, settings.fieldSeparator, this->tableIDs);
      aq::Logger::getInstance().log(AQ_NOTICE, "Load From Text AQ Matrix: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
    }

  }
  else if (mode == NESTED_2)
	{
    std::vector<std::string> files;
		if( aq::GetFiles( settings.szTempPath1, files ) != 0 )
			throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "");
		--selectLevel;
		char path[_MAX_PATH];
		sprintf( path, "%s_%d", settings.szTempPath1, selectLevel );
		//sprintf( path, "%s", settings.szTempPath1 );
		mkdir( path );
		for( size_t idx = 0; idx < files.size(); ++idx )
    {
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
				sprintf( oldFile, "%s/%s", settings.szTempPath1, files[idx].c_str() );
				
				char newFile[_MAX_PATH];
				std::string substr1 = files[idx].substr(0, 9).c_str();
				std::string substr2 = files[idx].substr(17, 13);
				sprintf( newFile, "%s/%s%s.tmp", path, substr1.c_str(), substr2.c_str() );
				
				remove( newFile );
				rename( oldFile, newFile );
			}
    }
		aq::DeleteFolder( settings.szTempPath1 );
	}
  else
  {
		throw aq::generic_error(aq::generic_error::NOT_IMPLEMENED, "");
	}

}

void AQEngine::generateAQMatrixFromPRM(const std::string tableName, aq::tnode * whereNode)
{
	Table::Ptr table = this->baseDesc.getTable(tableName);
	this->aqMatrix.reset(new aq::AQMatrix(this->settings));
	this->aqMatrix->simulate(table->TotalCount, 2);
	this->tableIDs.clear();
	this->tableIDs.push_back(table->ID);
}

// ------------------------------------------------------------------------------------------------
AQEngineWindows::AQEngineWindows(Base& _baseDesc, TProjectSettings& _settings)
  : AQEngine(_baseDesc, _settings)
{
}

int AQEngineWindows::run(const char * prg, const char * args) const
{    
  aq::Logger::getInstance().log(AQ_NOTICE, "call: '%s %s'\n", prg, args);
  int rc = 1;
  STARTUPINFOW si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  std::wstring wprg = aq::string2Wstring(prg);
  std::wstring warg = aq::string2Wstring(args);
  LPCWSTR prg_wstr = wprg.c_str();
  LPCWSTR arg_wstr = warg.c_str();
  if (CreateProcessW(prg_wstr, (LPWSTR)arg_wstr, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
  {
    rc = WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }
  return rc;
}

// ------------------------------------------------------------------------------------------------
AQEngineSystem::AQEngineSystem(Base& _baseDesc, TProjectSettings& _settings)
  : AQEngine(_baseDesc, _settings)
{
}

int AQEngineSystem::run(const char * prg, const char * args) const
{
  aq::Logger::getInstance().log(AQ_NOTICE, "call: '%s %s'\n", prg, args);
  return system((std::string(prg) + " " + std::string(args)).c_str());
}

}