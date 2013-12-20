#include "AQManager.h"
#include "AQEngineSimulate.h"

#include <aq/Database.h>
#include <aq/FileMapper.h>
#include <aq/Timer.h>

#include <aq/parser/SQLParser.h>
#include <aq/parser/sql92_grm_tab.hpp>
#include <aq/parser/ID2Str.h>

#include <aq/verbs/VerbNode.h>

#include <aq/db_loader/DatabaseLoader.h>

#include <aq/QueryResolver.h>
#include <aq/UpdateResolver.h>
#include <aq/ThesaurusReader.h>
#include <aq/TreeUtilities.h>
#include <aq/AQFunctor.h>

#include <fstream>

#include <boost/array.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/filesystem.hpp>

static boost::mutex parserMutex;

namespace fs = boost::filesystem;

// -------------------------------------------------------------------------------------------------
int check_database(const aq::Settings& settings)
{
  int rc = 0;
//  std::vector<std::string> errors;
//
//  aq::Logger::getInstance().log(AQ_NOTICE, "check base [%s]", settings.rootPath.c_str());
//
//  aq::Database db(settings.rootPath);
//  if (!db.isValid())
//  {
//    aq::Logger::getInstance().log(AQ_ERROR, "invalid database [%s]\n", settings.rootPath.c_str());
//    return EXIT_FAILURE;
//  }
//
//  aq::base_t b = db.getBaseDesc();
//
//  aq::ColumnItem item;
//  std::string value;
//  boost::shared_ptr<aq::ColumnMapper_Intf> m;
//  boost::shared_ptr<aq::ColumnMapper_Intf> tr;
//  for (auto& t : b.table)
//  {
//    aq::Logger::getInstance().log(AQ_NOTICE, "check table [id:%u;name:%s;cols:%u;records:%u]\n", t.id, t.name.c_str(), t.nb_cols, t.nb_record);
//    for (auto& c : t.colonne)
//    {
//      aq::Logger::getInstance().log(AQ_NOTICE, "check table [%u;%s] column [%u;%s] [records:%u]\n", t.id, t.name.c_str(), c.id, c.name.c_str(), t.nb_record);
//      for (size_t p = 0; p <= (t.nb_record / settings.packSize); p++)
//      {
//        std::string theFilename = aq::getThesaurusFileName(settings.dataPath.c_str(), t.id, c.id, p);
//        aq::Logger::getInstance().log(AQ_NOTICE, "check thesaurus [%s]\n", theFilename.c_str());
//        switch (c.type)
//        {
//        case aq::symbole::t_char:
//          tr.reset(new aq::ThesaurusReader<char, aq::FileMapper>(settings.dataPath.c_str(), t.id, c.id, c.size, settings.packSize, p, false));
//          break;
//        case aq::symbole::t_double: 
//          tr.reset(new aq::ThesaurusReader<double, aq::FileMapper>(settings.dataPath.c_str(), t.id, c.id, c.size, settings.packSize, p, false));
//          break;
//        case aq::symbole::t_int: 
//          tr.reset(new aq::ThesaurusReader<uint32_t, aq::FileMapper>(settings.dataPath.c_str(), t.id, c.id, c.size, settings.packSize, p, false));
//          break;
//        case aq::symbole::t_long_long: 
//        case aq::symbole::t_date1: 
//          tr.reset(new aq::ThesaurusReader<uint64_t, aq::FileMapper>(settings.dataPath.c_str(), t.id, c.id, c.size, settings.packSize, p, false));
//          break;
//        default:
//          aq::Logger::getInstance().log(AQ_ERROR, "type not supported [%s]\n", c.type);
//        }
//        size_t i = 0;
//        bool duplicatedValues = false;
//        std::set<std::string> values;
//        while (tr->loadValue(i++, item) != -1)
//        {
//          value = item.toString(tr->getType());
//          if (values.find(value) != values.end())
//          {
//            duplicatedValues = true;
//            aq::Logger::getInstance().log(AQ_ERROR, "duplicated values [%s] in thesaurus [%s]\n", value.c_str(), theFilename.c_str());
//          }
//          values.insert(value);
//        }
//        if (duplicatedValues)
//        {
//          std::stringstream ss;
//          ss << "duplicated values found in thesaurus [" << theFilename << "]";
//          errors.push_back(ss.str());
//        }
//        aq::Logger::getInstance().log(AQ_NOTICE, "%u unique values read in thesaurus [%s]\n", values.size(), theFilename.c_str());
//      }
//      aq::Logger::getInstance().log(AQ_NOTICE, "read full column\n");
//      switch (c.type)
//      {
//      case aq::symbole::t_char: 
//        m.reset(new aq::ColumnMapper<char, aq::FileMapper>(settings.dataPath.c_str(), t.id, c.id, c.size, settings.packSize, false)); 
//        break;
//      case aq::symbole::t_double: 
//        m.reset(new aq::ColumnMapper<double, aq::FileMapper>(settings.dataPath.c_str(), t.id, c.id, c.size, settings.packSize, false)); 
//        break;
//      case aq::symbole::t_int: 
//        m.reset(new aq::ColumnMapper<uint32_t, aq::FileMapper>(settings.dataPath.c_str(), t.id, c.id, c.size, settings.packSize, false)); 
//        break;
//      case aq::symbole::t_long_long: 
//      case aq::symbole::t_date1: 
//        m.reset(new aq::ColumnMapper<uint64_t, aq::FileMapper>(settings.dataPath.c_str(), t.id, c.id, c.size, settings.packSize, false)); 
//        break;
//      default:
//        aq::Logger::getInstance().log(AQ_ERROR, "type not supported [%s]\n", c.type);
//      }
//      for (size_t i = 0; i < t.nb_record; i++)
//      {
//        if (m->loadValue(i, item) != 0)
//        {
//          rc = EXIT_FAILURE;
//          aq::Logger::getInstance().log(AQ_ERROR, "bad index %u on table [%u;%s] column [%u;%s]\n", i, t.id, t.name.c_str(), c.id, c.name.c_str());
//          std::stringstream ss;
//          ss << "bad index " << i << " on table [" << t.id << ";" << t.name << "] column [" << c.id << ";" << c.name << "]";
//          errors.push_back(ss.str());
//        }
//        else if ((value = item.toString(m->getType())) == "")
//        {
//          rc = EXIT_FAILURE;
//          aq::Logger::getInstance().log(AQ_ERROR, "bad value on index %u on table [%u;%s] column [%u;%s]\n", i, t.id, t.name.c_str(), c.id, c.name.c_str());
//          std::stringstream ss;
//          ss << "bad value on index " << i << " on table [" << t.id << ";" << t.name << "] column [" << c.id << ";" << c.name << "]";
//          errors.push_back(ss.str());
//        }
//        else if ((i % ((t.nb_record / 10) + 1)) == 0)
//        {
//          aq::Logger::getInstance().log(AQ_INFO, "%s\n", value.c_str());
//        }
//      }
//    }
//  }
//
//  aq::Logger::getInstance().log(AQ_ERROR, "");
//  aq::Logger::getInstance().log(AQ_ERROR, "ERRORS:\n");
//
//  for (auto& error : errors)
//  {
//    aq::Logger::getInstance().log(AQ_ERROR, "%s\n", error.c_str());
//  }

  return rc;
}

// -------------------------------------------------------------------------------------------------
int process_aq_matrix(const std::string& query, const std::string& aqMatrixFileName, const std::string& answerFileName, aq::Settings& settings, aq::Base& baseDesc)
{
	aq::tnode	*pNode;
	int	nRet;

	//
	// Parse SQL request
	{
    
		boost::mutex::scoped_lock lock(parserMutex);
		aq::Logger::getInstance().log(AQ_INFO, "parse sql query: '%s'\n", query.c_str());
		if ((nRet = SQLParse(query.c_str(), pNode)) != 0 ) 
		{
			aq::Logger::getInstance().log(AQ_ERROR, "error parsing sql request '%s'\n", query.c_str());
			return EXIT_FAILURE;
		}

	}
	
  boost::array<uint32_t, 6> categories_order = { K_FROM, K_WHERE, K_SELECT, K_GROUP, K_HAVING, K_ORDER };
	aq::verb::VerbNode::Ptr spTree = aq::verb::VerbNode::BuildVerbsTree(pNode, categories_order, baseDesc, &settings );
	spTree->changeQuery();
	aq::util::cleanQuery( pNode );

	settings.answerFile = aqMatrixFileName;

	boost::shared_ptr<aq::AQMatrix> aqMatrix(new aq::AQMatrix(settings, baseDesc));
	std::vector<llong> tableIDs;
	
	aq::Timer timer;
	std::vector<std::string> answerFile;
	answerFile.push_back(aqMatrixFileName);
	for (std::vector<std::string>::const_iterator it = answerFile.begin(); it != answerFile.end(); ++it)
	{
		aqMatrix->load((*it).c_str(), tableIDs);
	}
	// aqMatrix->simulate(113867938, 2);
	// aqMatrix->simulate(10, 2);
	// tableIDs.push_back(8);
	aq::Logger::getInstance().log(AQ_INFO, "Load AQ Matrix: Elapsed Time = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());

	aq::AQEngineSimulate aqEngine(baseDesc, settings);
	aqEngine.setAQMatrix(aqMatrix);
	aqEngine.setTablesIDs(tableIDs);
  
  unsigned int id_generator = 1;
	aq::QueryResolver queryResolver(pNode, &settings, &aqEngine, baseDesc, id_generator);
  queryResolver.solveAQMatrix(spTree);

	return 0;
}

// -------------------------------------------------------------------------------------------------
int transform_query(const std::string& query, aq::Settings& settings, aq::Base& baseDesc)
{
	aq::tnode	*pNode;
	int	nRet;

	//
	// Parse SQL request
	{

		boost::mutex::scoped_lock lock(parserMutex);
		aq::Logger::getInstance().log(AQ_INFO, "parse sql query %s\n", query.c_str());
		if ((nRet = SQLParse(query.c_str(), pNode)) != 0 ) 
		{
			aq::Logger::getInstance().log(AQ_ERROR, "error parsing sql request '%s'\n", query.c_str());
			return EXIT_FAILURE;
		}

	}
  
  boost::array<uint32_t, 6> categories_order = { K_FROM, K_WHERE, K_SELECT, K_GROUP, K_HAVING, K_ORDER };
	aq::verb::VerbNode::Ptr spTree = aq::verb::VerbNode::BuildVerbsTree(pNode, categories_order, baseDesc, &settings );
	spTree->changeQuery();
	aq::util::cleanQuery( pNode );
	
	std::string str;
	aq::syntax_tree_to_aql_form(pNode, str);
  aq::ParseJeq( str );

	return 0;
}

// -------------------------------------------------------------------------------------------------
int load_database(const aq::Settings& settings, aq::base_t& baseDesc, const std::string& tableNameToLoad)
{
  aq::DatabaseLoader loader(baseDesc, settings.aqLoader, settings.rootPath, settings.packSize, ','/*settings.fieldSeparator*/, settings.csvFormat); // FIXME
  // aq::DatabaseLoader loader(baseDesc, settings.rootPath, settings.packSize',', settings.csvFormat);
  loader.generate_ini();
  if (tableNameToLoad != "")
  {
    for (size_t t = 0; t < baseDesc.table.size(); ++t)
    {
      if (tableNameToLoad != baseDesc.table[t].name)
      {
        continue;
      }
      aq::Logger::getInstance().log(AQ_INFO, "loading table %d\n", t + 1);
      assert(baseDesc.table[t].id = (int)(t + 1));
      loader.load(t + 1);
    }
  }
  else
  {
    loader.load();
  }
  return EXIT_SUCCESS;
}

// -------------------------------------------------------------------------------------------------
int generate_tmp_table(const aq::Settings& settings, aq::base_t& baseDesc, unsigned int nbValues, unsigned int minValue, unsigned int maxValue)
{
  if (minValue > maxValue)
    return -1;
  size_t tableIndex = baseDesc.nb_tables + 1;
  size_t columnIndex = 1;
  size_t partIndex = 0;
  size_t size = 0;
  std::string type;
  unsigned int range = maxValue - minValue;
  unsigned int value = 0;
  switch (sizeof(value))
  {
  case 4:  type = "INT"; size = 4; break;
  case 8:  type = "LON"; size = 8; break;
  default: type = "INT"; size = 4; break;
  }
  ::srand((unsigned int)::time(NULL));
  FILE * fd = NULL;
  for (size_t i = 0; i < nbValues; i++)
  {
    if ((i % settings.packSize) == 0)
    {
      if (fd != NULL)
        fclose(fd);
      std::string tmpFilename = settings.rootPath + aq::getTemporaryFileName(tableIndex,  columnIndex,  partIndex, type.c_str(),  size);
      fd = fopen(tmpFilename.c_str(), "w");
      if (fd == NULL)
        return -1;
    }
    value = minValue + (::rand() % range);
    fwrite(&value, sizeof(value), 1, fd);
  }
  fclose(fd);
  baseDesc.nb_tables += 1;
  baseDesc.table.push_back(aq::base_t::table_t());
  auto& table = *baseDesc.table.rbegin();
  std::stringstream ss;
  ss << "TMP" << tableIndex;
  table.name = ss.str();
  table.nb_cols = 1;
  table.id = (unsigned int)tableIndex;
  table.nb_record = nbValues;
  table.colonne.push_back(aq::base_t::table_t::col_t());
  auto& col = *table.colonne.rbegin();
  col.name = "V1";
  col.id = 1;
  col.size = 1;
  col.type = aq::symbole::t_long_long;
  return 0;
}

// -------------------------------------------------------------------------------------------------
int prepareQuery(const std::string& query, const aq::Settings& settingsBase, aq::Base& baseDesc, aq::Settings& settings, std::string& displayFile, const std::string queryIdentStr, bool force)
{		
	//
	// generate ident and ini file
  std::string queryIdentTmp = queryIdentStr;
	if (queryIdentTmp == "")
  {
    boost::uuids::uuid queryIdent = boost::uuids::random_generator()();
    std::ostringstream queryIdentOSS;
    queryIdentOSS << queryIdent;
    queryIdentTmp = queryIdentOSS.str();
    settings.changeIdent(queryIdentOSS.str());
  }
  else
  {
    settings.changeIdent(queryIdentTmp);
  }

	//
	// create directories
	std::list<fs::path> lpaths;
	lpaths.push_back(fs::path(settings.rootPath + "calculus/" + queryIdentTmp));
	lpaths.push_back(fs::path(settings.tmpPath));
	lpaths.push_back(fs::path(settings.dpyPath));
	for (std::list<fs::path>::const_iterator dir = lpaths.begin(); dir != lpaths.end(); ++dir)
	{
		if (fs::exists(*dir))
		{
      aq::Logger::getInstance().log(AQ_WARNING, "directory already exist '%s'\n", (*dir).string().c_str());
      if (!force)
      {
        return EXIT_FAILURE;
      }
		}
		else if (!fs::create_directory(*dir))
		{
			aq::Logger::getInstance().log(AQ_WARNING, "cannot create directory '%s'\n", (*dir).string().c_str());
      if (!force)
      {
        return EXIT_FAILURE;
      }
		}
	}

  //
  // write request file
  std::string queryFilename(settings.rootPath + "calculus/" + queryIdentTmp + "/Request.sql");
  std::ofstream queryFile(queryFilename.c_str());
  queryFile << query;
  queryFile.close();

	//
	// write ini file (it is needed for now by AQEngine)
	std::ofstream iniFile(settings.iniFile.c_str());
	settings.writeAQEngineIni(iniFile);
	iniFile.close();

	// generate answer file
	// displayFile = settings.szRootPath + "/calculus/" + queryIdentStr + "/display.txt"; // TODO
	displayFile = settings.rootPath + "calculus/" + queryIdentStr + "/answer.txt"; // TODO
	aq::Logger::getInstance().log(AQ_INFO, "save answer to %s\n", displayFile.c_str());

	return EXIT_SUCCESS;
}

// -------------------------------------------------------------------------------------------------
int processQuery(const std::string& query, aq::Settings& settings, aq::Base& baseDesc, aq::AQEngine_Intf * aq_engine,
                 const std::string& answer, bool keepFiles)
{
  int rc = EXIT_SUCCESS;

	try
	{
	
		aq::Logger::getInstance().log(AQ_INFO, "processing sql query\n");

		aq::tnode	*pNode  = nullptr;
		int	nRet;

		//
		// Parse SQL request
		{

			boost::mutex::scoped_lock lock(parserMutex);
			aq::Logger::getInstance().log(AQ_INFO, "parse sql query %s\n", query.c_str());
			if ((nRet = SQLParse(query.c_str(), pNode)) != 0 ) 
			{
				aq::Logger::getInstance().log(AQ_ERROR, "error parsing sql request '%s'\n", query.c_str());
				return EXIT_FAILURE;
			}

#if defined(_DEBUG) && defined(_TRACE)
			std::cout << *pNode << std::endl;
#endif

    }

    //
		// Transform SQL request in prefix form, 
    if (pNode->tag == K_SELECT)
    {
      unsigned int id_generator = 1;
      aq::Timer timer;
      aq::QueryResolver queryResolver(pNode, &settings, aq_engine, baseDesc, id_generator);
      aq::Table::Ptr result = queryResolver.solve();
      timer.stop();
      if (settings.cmdLine)
      {
        std::cout << queryResolver.getNbRows() << " rows processed in " << aq::Timer::getString(timer.getTimeElapsed()) << std::endl;
        std::cout << std::endl;
      }
    }
    else if (pNode->tag == K_UPDATE)
    {
      aq::UpdateResolver updateResolver(pNode, settings, aq_engine, baseDesc);
      updateResolver.solve();
    }
    else 
    {
      aq::Logger::getInstance().log(AQ_INFO, "[%s] is not supported", aq::id_to_string(pNode->tag));
    }

		delete pNode;
	}
	catch (const aq::generic_error& ge)
	{
		aq::Logger::getInstance().log(AQ_ERROR, "%s\n", ge.what());
		rc = EXIT_FAILURE;
	}
	catch (const std::exception& ex)
	{
		aq::Logger::getInstance().log(AQ_ERROR, "%s\n", ex.what());
		rc = EXIT_FAILURE;
	}
	catch (...)
	{
		aq::Logger::getInstance().log(AQ_ERROR, "unknown exception\n");
		rc = EXIT_FAILURE;
	}

  if (!keepFiles)
  {
    aq::Logger::getInstance().log(AQ_NOTICE, "remove temporary directory '%s'\n", settings.tmpPath.c_str());
    aq::DeleteFolder(settings.tmpPath.c_str());
    aq::Logger::getInstance().log(AQ_NOTICE, "remove working directory '%s'\n", settings.workingPath.c_str());
    aq::DeleteFolder(settings.workingPath.c_str());
  }

	return rc;
}

// -------------------------------------------------------------------------------------------------
int test_plugins(const std::string& plugins_path, const std::string& query, const aq::Settings& settings, const aq::Base& base)
{   

  aq::tnode * tree = nullptr;
  if (SQLParse(query.c_str(), tree) != 0 )
  {
    return EXIT_FAILURE;
  }

  std::cout << *tree << std::endl;

  try
  {
    aq::AQFunctor test(tree, plugins_path);
    test.dump(std::cout);
    test.callFunctor();
  }
  catch (const aq::generic_error& ge)
  {
    std::cerr << ge.what() << std::endl;
  }
  catch (const std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "UNCATCHED EXCEPTION" << std::endl;
    throw;
  }

  return EXIT_SUCCESS;
}