
/* $Id: time_types.cpp,v 1.7 2002/09/22 08:50:14 pstrand Exp $ */

#include "hw/compat.h"

#ifdef WIN32
# include "hw/windows.h"
#else
#include <ctime>     // Include before chrono to ensure C time functions are available
#include <thread>    // For std::this_thread::sleep_for
#include <chrono>    // For std::chrono::milliseconds
#endif

#include "hw/time_types.h"





namespace reaper
{
namespace hw
{
namespace time
{


TimeSpan TimeSpan::from_us(long us)
{
	return TimeSpan(0, us);
}

TimeSpan TimeSpan::from_ms(long ms) {
	return TimeSpan(ms / low_mod_ms, (ms % low_mod_ms) * 1000);
}

TimeSpan& TimeSpan::operator+=(const TimeSpan& t) {
	high += t.high;
	low += t.low;
	fixup();
	return *this;
}

TimeSpan& TimeSpan::operator-=(const TimeSpan& t) {
	high -= t.high;
	low -= t.low;
	fixup();
	return *this;
}

TimeSpan& TimeSpan::operator*=(double m)
{ 
	high = static_cast<long>(high * m);
	low = static_cast<long>(low * m);
	fixup();
	return *this;
}

TimeSpan operator+(const TimeSpan& t1, const TimeSpan& t2)
{
	return TimeSpan(t1.approx().upper() + t2.approx().upper(),
			t1.approx().lower() + t2.approx().lower());
}

TimeSpan operator-(const TimeSpan& t1, const TimeSpan& t2)
{
	return TimeSpan(t1.approx().upper() - t2.approx().upper(),
			t1.approx().lower() - t2.approx().lower());

}

#ifdef WIN32

void msleep(long t) {
	Sleep(t);
}

#else

void msleep(long t) {
	// Modernized: Using C++11 std::this_thread::sleep_for instead of nanosleep
	std::this_thread::sleep_for(std::chrono::milliseconds(t));
}

#endif

}
}
}
