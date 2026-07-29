#include "hw/abstime.h"
#include "hw/reltime.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <thread>

namespace reaper::hw::time
{

namespace {

TimeSpan& reference_offset()
{
	static TimeSpan offset;
	return offset;
}

TimeSpan& relative_time_source()
{
	static TimeSpan relative_time;
	return relative_time;
}

TimeSpan& pause_time()
{
	static TimeSpan paused_at;
	return paused_at;
}

bool local_time(std::time_t value, std::tm& result)
{
#ifdef _WIN32
	return localtime_s(&result, &value) == 0;
#else
	return localtime_r(&value, &result) != nullptr;
#endif
}

}

TimeSpan get_time()
{
	const auto elapsed = std::chrono::steady_clock::now().time_since_epoch();
	const auto microseconds =
		std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	return TimeSpan::from_us(microseconds) + reference_offset();
}

std::ostream& operator<<(std::ostream& output, const TimeSpan& time)
{
	return output << std::hex << time.approx().upper() << ':'
		      << time.approx().lower();
}

std::istream& operator>>(std::istream& input, TimeSpan& time)
{
	char separator;
	TimeRep high;
	TimeRep low;
	input >> std::hex >> high >> separator >> low;
	if (input && separator == ':')
		time = TimeSpan(high, low);
	else
		input.setstate(std::ios::failbit);
	return input;
}

void set_time(TimeSpan reference)
{
	reference_offset() = reference - get_time();
}

void reset_rel_time()
{
	relative_time_source() = {};
}

RelTime get_rel_time()
{
	return static_cast<RelTime>(relative_time_source().approx().to_ms());
}

void rel_time_tick(RelTime ticks)
{
	relative_time_source() += TimeSpan::from_ms(ticks);
}

void start_time()
{
	reference_offset() -= get_time() - pause_time();
}

void stop_time()
{
	pause_time() = get_time();
}

std::string strtime(const std::string& format)
{
	const std::string actual_format =
		format.empty() ? "%d %H:%M:%S" : format;
	const auto now = std::chrono::system_clock::now();
	const auto wall_time = std::chrono::system_clock::to_time_t(now);
	std::tm local{};

	if (!local_time(wall_time, local))
		return "time_error";

	std::ostringstream output;
	output << std::put_time(&local, actual_format.c_str());
	return output.str();
}

void msleep(long milliseconds)
{
	if (milliseconds > 0)
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

}
