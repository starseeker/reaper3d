#include "hw/time_types.h"

namespace reaper::hw::time
{

TimeSpan TimeSpan::from_us(TimeRep us)
{
	return TimeSpan(us);
}

TimeSpan TimeSpan::from_ms(TimeRep ms)
{
	return TimeSpan(ms * 1'000);
}

TimeSpan& TimeSpan::operator+=(const TimeSpan& other)
{
	microseconds += other.microseconds;
	return *this;
}

TimeSpan& TimeSpan::operator-=(const TimeSpan& other)
{
	microseconds -= other.microseconds;
	return *this;
}

TimeSpan& TimeSpan::operator*=(double multiplier)
{
	microseconds = static_cast<TimeRep>(microseconds * multiplier);
	return *this;
}

TimeSpan operator+(TimeSpan lhs, const TimeSpan& rhs)
{
	lhs += rhs;
	return lhs;
}

TimeSpan operator-(TimeSpan lhs, const TimeSpan& rhs)
{
	lhs -= rhs;
	return lhs;
}

}
