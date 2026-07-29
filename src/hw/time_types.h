#ifndef REAPER_HW_TIME_TYPES_H
#define REAPER_HW_TIME_TYPES_H

#include <cstdint>

namespace reaper::hw::time
{

using TimeRep = std::int64_t;

inline constexpr TimeRep low_mod_us = 1'000'000'000;
inline constexpr TimeRep low_mod_ms = low_mod_us / 1'000;

class TimeApprox {
	TimeRep microseconds;

public:
	explicit TimeApprox(TimeRep value)
		: microseconds(value)
	{
	}

	operator double() const { return static_cast<double>(microseconds); }

	TimeRep to_us() const { return microseconds; }
	TimeRep to_ms() const { return microseconds / 1'000; }
	double to_s() const { return static_cast<double>(microseconds) / 1'000'000.0; }

	TimeRep upper() const { return microseconds / low_mod_us; }
	TimeRep lower() const { return microseconds % low_mod_us; }
};

/** A microsecond-resolution time difference relative to an unspecified epoch. */
class TimeSpan {
	TimeRep microseconds = 0;

	explicit TimeSpan(TimeRep value)
		: microseconds(value)
	{
	}

public:
	TimeSpan() = default;
	TimeSpan(TimeRep high, TimeRep low)
		: microseconds(high * low_mod_us + low)
	{
	}

	static TimeSpan from_us(TimeRep us);
	static TimeSpan from_ms(TimeRep ms);

	TimeApprox approx() const { return TimeApprox(microseconds); }

	TimeSpan& operator+=(const TimeSpan& other);
	TimeSpan& operator-=(const TimeSpan& other);
	TimeSpan& operator*=(double multiplier);

	bool operator==(const TimeSpan& other) const
	{
		return microseconds == other.microseconds;
	}
	bool operator!=(const TimeSpan& other) const { return !(*this == other); }
	bool operator<(const TimeSpan& other) const
	{
		return microseconds < other.microseconds;
	}
	bool operator>(const TimeSpan& other) const
	{
		return microseconds > other.microseconds;
	}

	friend TimeSpan operator+(TimeSpan lhs, const TimeSpan& rhs);
	friend TimeSpan operator-(TimeSpan lhs, const TimeSpan& rhs);
};

TimeSpan operator+(TimeSpan lhs, const TimeSpan& rhs);
TimeSpan operator-(TimeSpan lhs, const TimeSpan& rhs);

/// Relative time in milliseconds.
using RelTime = long;

}

#endif
