#include <boost/test/unit_test.hpp>
#include <aq/DateConversion.h>

BOOST_AUTO_TEST_SUITE(DateConversion)

BOOST_AUTO_TEST_CASE(test_1)
{
  aq::DateConversion dateConverter;
  const std::string date_ref("1981-05-30 18:36:23");
  const long long int_date_ref = dateConverter.dateToBigInt(date_ref.c_str());

  // input
  const std::string date("30/05/1981 18:36:23");
  dateConverter.setInputFormat("%d/%m/%Y %H:%M:%S");
  dateConverter.setOutputFormat("%d/%m/%Y %H:%M:%S"); 
    
  const long long aq_int_date = dateConverter.dateToBigInt(date.c_str());
  BOOST_CHECK(aq_int_date == int_date_ref);

  const std::string new_date_str = dateConverter.bigIntToDate(aq_int_date);
  BOOST_CHECK(new_date_str == date);
}

BOOST_AUTO_TEST_SUITE_END()
