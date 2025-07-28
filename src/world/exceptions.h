
/* $Id: exceptions.h,v 1.17 2002/08/12 11:39:39 pstrand Exp $ */


#ifndef REAPER_WORLD_EXCEPTIONS_H
#define REAPER_WORLD_EXCEPTIONS_H 

#include "hw/compat.h"
#include "main/exceptions.h"

namespace reaper
{
namespace world
{

define_reaper_exception(world_init_error);
define_reaper_exception(world_existing);
define_reaper_exception(world_nonexistant);
define_reaper_exception(world_format_error);
define_reaper_exception(world_error);

}
}

#endif

