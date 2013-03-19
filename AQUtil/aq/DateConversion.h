#ifndef __AQ_DATE_CONVERSION_H__
#define __AQ_DATE_CONVERSION_H__

namespace aq
{

typedef enum 
{
	DDMMYYYY_HHMMSS,
	DDMMYYYY,
	DDMMYY,
	DDMMYYYY_HHMMSS_MMM
}DateType;

//1 - success, 0 - failure
int dateToBigInt( const char* strval, DateType dateType, long long * intval );
int bigIntToDate( long long intval, DateType dateType, char* strval );

}

#endif