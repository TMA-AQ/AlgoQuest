#ifndef __AQ_DATE_CONVERSION_H__
#define __AQ_DATE_CONVERSION_H__

#include <cstdint>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace aq
{

class DateConversion
{

public:

  typedef enum 
  {
    ISO_EXTENDED
  } DateType;

  DateConversion();
  DateConversion(const char * format);
  ~DateConversion();

  void setInputFormat(const char * format) { this->input_facet->format(format); }
  void setOutputFormat(const char * format) { this->output_facet->format(format); }

  /// 1 - success, 0 - failure
  long long dateToBigInt(const char * strval);
  long long dateToBigInt(const char * strval, const char * format);
  std::string bigIntToDate(long long intval);
  std::string bigIntToDate(long long intval, const char * format);
  
  static long long currentDate();

private:;
  DateConversion(const DateConversion& o); // not implemented
  DateConversion& operator=(const DateConversion& o); // not implemented

  int dateToBigInt( const char* strval, DateType dateType, long long* intval );
  int bigIntToDate( long long intval, DateType dateType, char* strval );
  
  std::stringstream ssInput, ssOutput;
  boost::posix_time::time_input_facet * input_facet, * convert_input_facet;
  boost::posix_time::time_facet * output_facet, * convert_output_facet;
};

}

#endif