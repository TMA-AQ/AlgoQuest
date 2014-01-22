#include <aq/Settings.h>
#include <aq/Base.h>
#include <aq/AQEngine_Intf.h>

/// TODO
int check_database(const aq::Settings::Ptr settings);

/// TODO
int process_aq_matrix(const std::string & query, 
                      const std::string & aqMatrixFileName, 
                      const std::string & answerFileName, 
                      aq::Settings::Ptr   settings, 
                      aq::Base::Ptr       baseDesc);

/// TODO
int transform_query(const std::string & query, 
                    aq::Settings::Ptr   settings, 
                    aq::Base::Ptr       baseDesc);

/// TODO
int load_database(const aq::Settings::Ptr   settings, 
                  aq::base_t              & baseDesc, 
                  const std::string       & tableNameToLoad);

/// TODO
int generate_tmp_table(const aq::Settings::Ptr   settings, 
                       aq::base_t              & baseDesc, 
                       unsigned int              nbValues, 
                       unsigned int              minValue, 
                       unsigned int              maxValue);

/// TODO
int prepareQuery(const std::string       & query, 
                 const aq::Settings::Ptr   settingsBase, 
                 aq::Base::Ptr             baseDesc, 
                 aq::Settings::Ptr         settings, 
                 std::string             & displayFile, 
                 const std::string    queryIdentStr, 
                 bool                 force);

/// TODO
int processQuery(const std::string              & query, 
                 aq::Settings::Ptr                settings, 
                 aq::Base::Ptr                    baseDesc, 
                 aq::engine::AQEngine_Intf::Ptr   aq_engine,
                 const std::string              & answer, 
                 bool                             keepFiles);

/// TODO
int test_plugins(const std::string       & plugins_path,
                 const std::string       & query, 
                 const aq::Settings::Ptr   settings, 
                 const aq::Base::Ptr       base);
