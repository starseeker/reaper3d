
/* $Id: time.h,v 1.40 2002/01/31 05:36:37 peter Exp $ */

#ifndef REAPER_HW_TIME_H
#define REAPER_HW_TIME_H

#include "hw/time_types.h"

#include <string>

namespace reaper
{
namespace hw
{
namespace time
{


std::ostream& operator<<(std::ostream& os, const TimeSpan& t);
std::istream& operator>>(std::istream& is, TimeSpan& t);



/// sleep t milliseconds (not exact)
void msleep(RelTime t);

/** Get time in microseconds.
    Relative to something unspecified (use diffs...)
 */
TimeSpan get_time();

/** Set reference time. */
void set_time(TimeSpan ref);


/// Reset relative time to zero.
void reset_rel_time();

/// Get current relative time.
RelTime get_rel_time();

/// Advance relative time
void rel_time_tick(RelTime ticks);

/** Current time in string representation. */
std::string strtime(const std::string& format = "");

}
}
}
#endif

