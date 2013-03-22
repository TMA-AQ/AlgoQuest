#ifndef __AQ_DATE_CONVERSION_H__
#define __AQ_DATE_CONVERSION_H__

namespace aq
{

typedef enum 
{
	DDMMYYYY_HHMMSS,
	DDMMYYYY,
	DDMMYY,
	DDMMYYYY_HHMMSS_MMM,
	YYYYMM
}DateType;

//1 - success, 0 - failure
int dateToBigInt( const char* strval, DateType dateType, long long* intval );
int dateToBigInt( const char* strval, DateType* dateType, long long* intval );
int bigIntToDate( long long intval, DateType dateType, char* strval );
void dateToParts(	long long intval, int& year, int& month, int& day, int& hour, 
					int& minute, int& second, int& millisecond );
void dateFromParts(	long long& intval, int year, int month, int day, int hour, 
					int minute, int second, int millisecond );
long long currentDate();
}

#endif