#include <aq/TestRunner.h>
#include <aq/MySQLHelper.h>
#include <aq/QueryGenerator.h> 
#include <aq/VerbBuilder.h>
#include <aq/AQLParser.h>
#include <aq/Timer.h>
#include <aq/Logger.h>

#include <fstream>
#include <sstream>

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#if defined(_DEBUG)
#include <vld.h>
#endif

namespace po = boost::program_options;

int yyerror(char const * error)
{
  std::cerr << error << std::endl;
  return 0;
}

bool in_path(const std::vector<std::string>& paths, const std::string& filename)
{
  std::string suffix;
  std::string::size_type pos = filename.find(".");
  if (pos == std::string::npos)
  {
    suffix = ".exe";
  }
  for (const auto& p : paths)
  {
    auto f = boost::filesystem::path(p) / boost::filesystem::path(filename + suffix);
    if (boost::filesystem::exists(f))
    {
      aq::Logger::getInstance().log(AQ_INFO, "using [%s]\n", boost::filesystem::canonical(f).string().c_str());
      return true;
    }
  }
  return false;
}

struct Runner : public aq::DatabaseGenerator::handle_t
{
  Runner()
    : opt(new aq::TestCase::opt_t), nb_tests(1)
  {
  }

  int initialize(int argc, char ** argv)
  {
    try
    {
      std::string log_ident;
      unsigned int log_level;
      bool log_lock_mode;
      bool log_date_mode;
      bool log_pid_mode;

      size_t workers;
      size_t aq_process_threads;

      std::string point_mode;
      std::string gen_mode;
      std::string value_mode;

      boost::filesystem::path iniFile("aq.ini");
      if (boost::filesystem::exists(iniFile))
      {
        std::ifstream fin("aq.ini");
        opt->parse(fin);
      }

      //
      // look for properties file in args
      for (size_t i = 1; i < argc; i++)
      {
        if ((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--settings") == 0))
        {
          if ((i + 1) < argc)
          {
            std::ifstream fin(argv[i+1]);
            opt->parse(fin);
          }
        }

      }

      //
      // command line arguments are prior to settings file
      po::options_description all("Allowed options");
      all.add_options()
        ("help,h", "produce help message")
        ("settings,s", "")
        ;

      po::options_description log_options("Logging");
      log_options.add_options()
        ("log-level,v", po::value<unsigned int>(&log_level)->default_value(AQ_LOG_WARNING), "CRITICAL(2), ERROR(3), WARNING(4), NOTICE(5), INFO(6), DEBUG(7)")
        ("log-lock", po::bool_switch(&log_lock_mode), "for multithread program")
        ("log-date", po::bool_switch(&log_date_mode), "add date to log")
        ("log-pid", po::bool_switch(&log_pid_mode), "add thread id to log")
        ("log-ident", po::value<std::string>(&log_ident)->default_value("aq_query_resolver"), "")
        ;

      po::options_description algoquest("AlgoQuest");
      algoquest.add_options()
        ("aq-home,r", po::value<std::string>(&opt->aq_path)->default_value(opt->aq_path), "set AQ Home (AQ_HOME environment variable)")
        ("aq-name,n", po::value<std::string>(&opt->aq_name)->default_value(opt->aq_name), "")
        ("aq-engine,e", po::value<std::string>(&opt->aq_engine)->default_value(opt->aq_engine), "")
        ("aq-loader,l", po::value<std::string>(&opt->aq_loader)->default_value(opt->aq_loader), "")
        ("worker,w", po::value<size_t>(&workers), "number of thread assigned to resolve the bunch of sql queries")
        ("paralell,p", po::value<size_t>(&aq_process_threads), "number of thread assigned resolve one sql queries")
        ;

      po::options_description mysql("MySQL");
      mysql.add_options()
        ("mysql-host", po::value<std::string>(&opt->mysql_host)->default_value(opt->mysql_host), "")
        ("mysql-user", po::value<std::string>(&opt->mysql_user)->default_value(opt->mysql_user), "")
        ("mysql-pass", po::value<std::string>(&opt->mysql_pass)->default_value(opt->mysql_pass), "")
        ("mysql-name", po::value<std::string>(&opt->mysql_name)->default_value(opt->mysql_name), "")
        ;
      
      po::options_description generator("Generator");
      generator.add_options()
        ("query-generator", po::value<std::string>(&opt->generator_filename)->default_value(opt->generator_filename), "")
        ("n-tables", po::value<size_t>(&opt->nb_tables)->default_value(opt->nb_tables), "")
        ("n-rows", po::value<size_t>(&opt->nb_rows)->default_value(opt->nb_rows), "")
        ("min-value", po::value<int>(&opt->min_value)->default_value(opt->min_value), "")
        ("max-value", po::value<int>(&opt->max_value)->default_value(opt->max_value), "")
        ("point-mode", po::value<std::string>(&point_mode)->default_value("MIN_MAX"), "[FULL|MIN_MAX]")
        ("gen-mode", po::value<std::string>(&gen_mode)->default_value("INTERSECT"), "[ALL|INTERSECT]")
        ("value-mode", po::value<std::string>(&value_mode)->default_value("RANDOM"), "[ALL_UNIQUE|ALL_MULTIPLE|RANDOM]")
        ;

      all.add(log_options).add(algoquest).add(mysql).add(generator);

      po::variables_map vm;
      po::store(po::command_line_parser(argc, argv).options(all).run(), vm);
      po::notify(vm);    

      if (vm.count("help"))
      {
        std::cerr << all << "\n";
        return EXIT_FAILURE;
      }

      aq::Logger::getInstance().setLevel(log_level);
      aq::verb::VerbFactory::GetInstance().setBuilder(new aq::VerbBuilder);

      opt->point_mode = (point_mode == "FULL") ? aq::DatabaseGenerator::point_mode_t::FULL : aq::DatabaseGenerator::point_mode_t::MIN_MAX;
      opt->gen_mode = (gen_mode == "ALL") ? aq::DatabaseGenerator::gen_mode_t::ALL : aq::DatabaseGenerator::gen_mode_t::INTERSECT;
      opt->value_mode = 
        (value_mode == "ALL") ? aq::DatabaseGenerator::value_mode_t::ALL_UNIQUE :
        (value_mode == "ALL_UNIQUE") ? aq::DatabaseGenerator::value_mode_t::ALL_MULTIPLE :
        aq::DatabaseGenerator::value_mode_t::RANDOM;
      
      // check files
      char * path = ::getenv("PATH");
      std::vector<std::string> paths;
      boost::split(paths, path, boost::is_any_of(";"));
      std::string files[] = { opt->aq_engine, opt->aq_loader };
      for (const auto& f : files)
      {
        if (!boost::filesystem::exists(boost::filesystem::path(f)) && !in_path(paths, f))
        {
          std::cerr << "cannot find " << f << std::endl;
          return EXIT_FAILURE;
        }
      }
      
      if (!boost::filesystem::exists(boost::filesystem::path(opt->generator_filename.c_str())))
      {
          std::cerr << "cannot find " << opt->generator_filename << std::endl;
          return EXIT_FAILURE;
      }

      std::ostringstream oss;
      for (size_t i = 1; i <= opt->nb_tables; i++)
      {
        oss.str("");
        oss << "t" << i;
        tables.push_back(oss.str());
      }

      gen.reset(new aq::DatabaseGenerator(tables, opt->nb_rows, opt->min_value, opt->max_value, opt->point_mode, opt->gen_mode, opt->value_mode));

      tc.reset(new aq::TestCase);
      aq::Settings settings;
      settings.initPath(opt->aq_path + "/" + opt->aq_name);
      settings.changeIdent("test");
      settings.aqEngine = opt->aq_engine;
      settings.aqLoader = opt->aq_loader;
      boost::shared_ptr<aq::DatabaseIntf> db1(new aq::AlgoQuestDatabase(settings, true));
      boost::shared_ptr<aq::DatabaseIntf> db2(new aq::MySQLDatabase(opt->mysql_host, opt->mysql_user, opt->mysql_pass, opt->mysql_name));
      tc->add(db1);
      tc->add(db2);

      std::fstream fin(opt->generator_filename.c_str());  
      this->queriesGenerators.clear();
      while (!fin.eof())
      {   
        std::string genQuery;
        aq::QueryGenerator::ops_t ops;
        aq::QueryGenerator::idents_t idents;
        aq::QueryGenerator::parse(fin, genQuery, ops, idents);
        if (genQuery != "")
        {
          boost::shared_ptr<aq::QueryGenerator> gen(new aq::QueryGenerator(genQuery, ops, idents));
          this->queriesGenerators.push_back(gen);
        }
      }
    }
    catch (const boost::program_options::error& error)
    {
      std::cerr << error.what();
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }

  int run()
  {
    gen->getNbDB();
    gen->generate(this);
    return EXIT_SUCCESS;
  }

  void push(const handle_t::tables_t& tables)
  { 
    aq::Timer timer;
    std::ostringstream msg;
    size_t col = 0;
    tc->clean();
    std::cout << "Test " << nb_tests++ << std::endl;
    std::cout << "Tables's Ranges: " << std::endl;
    for (const auto& table : tables)
    {
      msg.str("");
      auto min = *table.second.begin();
      auto max = *table.second.rbegin();
      std::cout << table.first << " = [" << min << ";" << max << "] => { ";
      std::copy(table.second.begin(), table.second.end(), std::ostream_iterator<int>(std::cout, " "));
      std::cout << " }" << std::endl;
      col += 1;

      tc->createTable(table.first);
      tc->insertValues(table);
    }

    std::string aqlQuery;
    aq::DatabaseIntf::result_t result;
    size_t nb_queries = 0;
    for (const auto& gen : queriesGenerators) 
    {
      gen->reset();
      nb_queries += gen->getNbQueries();
    }
    for (auto& gen : queriesGenerators)
    {
      while ((aqlQuery = gen->next()) != "")
      {
        aq::core::SelectStatement ss;
        aq::parser::parse(aqlQuery, ss);

        std::string sql_query;
        ss.setOutput(aq::core::SelectStatement::output_t::SQL);
        ss.to_string(sql_query);

        if (sql_query.find("full outer join") != std::string::npos)
        {
          assert(nb_queries > 0);
          nb_queries -= 1;
          continue;
        }

        if (!tc->execute(ss, result)) 
        { 
          std::cout << "E";
        }
        else 
        { 
          std::cout << ".";
        }
      }
    }
    std::cout 
      << std::endl 
      << nb_queries << " queries checked [ sucess => " << tc->getNbSuccess() 
      << " | failure => " << tc->getNbFailure() 
      << " | average row => " << tc->getNbResult()
      << " ] in " << aq::Timer::getString(timer.getTimeElapsed())
      << std::endl;
    assert((tc->getNbSuccess() + tc->getNbFailure()) == nb_queries);
    assert(tc->getNbTests() == nb_queries);
  }

  boost::shared_ptr<aq::TestCase::opt_t> opt;
  std::list<std::string> tables;
  boost::shared_ptr<aq::DatabaseGenerator> gen;
  boost::shared_ptr<aq::TestCase> tc;
  std::list<boost::shared_ptr<aq::QueryGenerator> > queriesGenerators;
  size_t nb_tests;
};

int main(int argc, char * argv[])
{
  int rc = 0;
  Runner runner;
  if ((rc = runner.initialize(argc, argv)) == EXIT_SUCCESS)
  {
    aq::Timer timer;
    rc = runner.run();
    std::cout << runner.nb_tests << " databases tests performed in " << aq::Timer::getString(timer.getTimeElapsed()) << std::endl;
  }
	return rc;
}

