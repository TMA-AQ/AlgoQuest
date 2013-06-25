#include "AQEngine.h"
#include <aq/Exceptions.h>
#include "parser/JeqParser.h"
#include "SQLPrefix.h"
#include "TreeUtilities.h"
#include <string>
#include <aq/Logger.h>
#include <aq/Timer.h>
#include <boost/date_time/posix_time/posix_time.hpp>

#ifdef WIN32
#include <direct.h>
#endif

namespace aq
{

  AQEngine::AQEngine(Base& _baseDesc, TProjectSettings& _settings)
    : baseDesc(_baseDesc), settings(_settings)
  {
  }


  AQEngine::~AQEngine(void)
  {
  }

  void AQEngine::call(const std::string& query, mode_t mode)
  {
    if (query.size() < 2048) // fixme
      aq::Logger::getInstance().log(AQ_DEBUG, "\n%s\n", query.c_str());

    //
    //
    aq::SaveFile( settings.szOutputFN, query.c_str() );

    //

#ifdef WIN32
    // create folders for the engine
    // mkdir( settings.szTempPath1 );
    mkdir( settings.szTempPath2 );
#endif

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
      timer.start();
      tableIDs.clear();
      aqMatrix.reset(new aq::AQMatrix(settings, baseDesc));
    
      if (settings.useBinAQMatrix)
      {
        aqMatrix->load(settings.szTempPath2, this->tableIDs);
        aq::Logger::getInstance().log(AQ_NOTICE, "Load From Binary AQ Matrix: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
      }
      else
      {
        aqMatrix->load(settings.szAnswerFN, settings.fieldSeparator, this->tableIDs);
        aq::Logger::getInstance().log(AQ_NOTICE, "Load From Text AQ Matrix: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
      }
    
      if (mode == REGULAR)
      {
        aq::DeleteFolder( settings.szTempPath2 );
      }
      else
      {
        aq::CleanFolder( settings.szTempPath1 );
      }

    }
    else if (mode == NESTED_2)
    {
    }
    else
    {
      throw aq::generic_error(aq::generic_error::NOT_IMPLEMENED, "");
    }

  }

  void AQEngine::call(aq::tnode *pNode, mode_t mode, int selectLevel)
  {
    std::string query;
    aq::syntax_tree_to_prefix_form( pNode, query );

    // a group is an order in aq engine => change ORDER by GROUP
    std::string::size_type pos = query.find("GROUP");
    if (pos != std::string::npos)
    {
      query.replace(pos, 5, "ORDER");
    }

    pos = query.find("ORDER");
    if (pos != std::string::npos)
    {
      std::string queryTmp = query.substr(0, pos);
      std::string group = query.substr(pos);
      ParseJeq( queryTmp );
      query = queryTmp;
      if (group.size() > 5) // check if Group By is not empty
        query += group;
    }
    else
    {
      ParseJeq( query );
    }
	
    this->call(query, mode);
  }

  void AQEngine::generateAQMatrixFromPRM(const std::string tableName, aq::tnode * whereNode)
  {
    Table::Ptr table = this->baseDesc.getTable(tableName);
    this->aqMatrix.reset(new aq::AQMatrix(this->settings, this->baseDesc));
    //this->aqMatrix->simulate(table->TotalCount, 2);
    this->tableIDs.clear();
    this->tableIDs.push_back(table->ID);
  }

#ifdef WIN32

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

#endif

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
