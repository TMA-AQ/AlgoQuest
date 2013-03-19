#include "Timer.h"

using namespace aq;

Timer::Timer()
: begin(boost::posix_time::microsec_clock::local_time()),
	end(boost::posix_time::neg_infin)
{
}

	
Timer::~Timer()
{
}

void Timer::start()
{
	begin = boost::posix_time::microsec_clock::local_time();
}

void Timer::stop()
{
	end = boost::posix_time::microsec_clock::local_time();
}

boost::posix_time::time_duration Timer::getTimeElapsed() const
{
	boost::posix_time::time_duration time_elapsed;
	if (end > begin)
	{
		time_elapsed = end - begin;
	}
	else
	{
		time_elapsed = boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time()) - begin;
	}
	return time_elapsed;
}

const std::string Timer::getString(boost::posix_time::time_duration duration)
{
	std::ostringstream oss;
	oss << duration;
	return oss.str();
}