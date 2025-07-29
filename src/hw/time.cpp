/* $Id: time.cpp,v 1.55 2002/03/11 10:50:48 pstrand Exp $ */

#include "hw/compat.h"

// Keep time.h for system compatibility, then add modern C++ headers
#include <ctime>   // C time functions in std namespace  
#include <chrono>  // Modern C++11 time handling
#include <limits>
#include <iostream>

#ifdef WIN32
# include "hw/windows.h"
#else
# include <stdio.h>
# include <sys/time.h>
# ifndef __USE_BSD
#  define __USE_BSD
# endif
# include <unistd.h>
#endif

#include "hw/reltime.h"
#include "hw/abstime.h"

namespace reaper
{
namespace hw
{
namespace time
{

TimeSpan& ref_diff() {
	static TimeSpan diff(0,0);
	return diff;
}

#ifdef WIN32


float get_freqscale()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return static_cast<float>(1e6 / freq.QuadPart);
}

TimeSpan get_time() {
	static float scale = get_freqscale();
	LARGE_INTEGER tm;
	QueryPerformanceCounter(&tm);
	tm.QuadPart *= scale;
	return TimeSpan(tm.QuadPart / low_mod_us, tm.QuadPart % low_mod_us) + ref_diff();
}


#else

TimeSpan get_time() {
	struct timeval t;
	gettimeofday(&t, 0);

	unsigned long high = t.tv_sec / 1000;
	unsigned long low = t.tv_sec % 1000 * 1000000 + t.tv_usec;
	TimeSpan res(high, low);

	return res + ref_diff();
}


#endif



std::ostream& operator<<(std::ostream& os, const TimeSpan& t)
{
	os << std::hex << t.approx().upper() << ":" << t.approx().lower();
	return os;
}

std::istream& operator>>(std::istream& is, TimeSpan& t)
{
	char junk;
	long high, low;
	is >> std::hex >> high >> junk >> low;
	t = TimeSpan(high, low);
	return is;
}


void set_time(TimeSpan ref)
{	
	ref_diff() = ref - get_time(); 
}


namespace {
	TimeSpan& rel_time_source() {
		static TimeSpan rel_time(0,0);
		return rel_time;
	}
	TimeSpan& pause() {
		static TimeSpan pt(0,0);
		return pt;
	}
}

void reset_rel_time()
{
	rel_time_source() = TimeSpan(0,0);
}

RelTime get_rel_time()
{
	return rel_time_source().approx().to_ms();
}

void rel_time_tick(RelTime ticks)
{
	rel_time_source() += TimeSpan::from_ms(ticks);
}

void start_time()
{
	ref_diff() -= get_time() - pause();
}

void stop_time()
{
	pause() = get_time();
}

std::string strtime(const std::string& format)
{
	// Modernized: Using C++11 std::chrono instead of legacy C time.h
	std::string fmt = format.empty() ? "%d %H:%M:%S" : format;
	
	// Get current time using std::chrono::system_clock
	auto now = std::chrono::system_clock::now();
	auto time_t_now = std::chrono::system_clock::to_time_t(now);
	
	// Format using C time functions (still needed for strftime)
	std::tm* local_tm = ::localtime(&time_t_now);
	
	static char buf[100];
	::strftime(buf, 100, fmt.c_str(), local_tm);
	return std::string(buf);
}


}
}
}
