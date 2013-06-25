#ifndef __AQ_TIMER_H__
#define __AQ_TIMER_H__

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace aq
{

class Timer
{
public:
	Timer();
	~Timer();

	void start();
	void stop();
	boost::posix_time::time_duration getTimeElapsed() const;
	static const std::string getString(boost::posix_time::time_duration);

private:
	boost::posix_time::ptime begin;
	boost::posix_time::ptime end;
};

}

#endif
