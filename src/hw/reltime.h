
/* $Id: reltime.h,v 1.1 2002/03/11 10:51:16 pstrand Exp $ */

#ifndef REAPER_HW_RELTIME_H
#define REAPER_HW_RELTIME_H

#include "hw/time_types.h"

#include <string>

namespace reaper
{
namespace hw
{
namespace time
{


/// Reset relative time to zero.
void reset_rel_time();

/// Get current relative time.
RelTime get_rel_time();

/// Advance relative time
void rel_time_tick(RelTime ticks);

/** Current time in string representation. */
std::string strtime(const std::string& format);

}
}
}
#endif

