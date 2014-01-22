#ifndef __AQ_TIMER_H__
#define __AQ_TIMER_H__

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace aq
{

/// \brief helper class to report prerformance
class Timer
{
public:
  Timer(); ///< \brief Start the timer
  ~Timer();

	void start(); ///< \brief Start the timer
	void stop(); ///< \brief Stop the timer

  /// \brief get time elapsed
  /// \return time elapsed
	boost::posix_time::time_duration getTimeElapsed() const;

  /// \brief get string representation of a duration
  /// \param d the duration
  /// \return the string representation
	static const std::string getString(boost::posix_time::time_duration d);

private:
	boost::posix_time::ptime begin;
	boost::posix_time::ptime end;
};

}

#endif
