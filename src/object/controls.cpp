/*
 * $Author: peter $
 * $Date: 2001/12/09 19:03:36 $
 * $Log: controls.cpp,v $
 * Revision 1.4  2001/12/09 19:03:36  peter
 * doh
 *
 * Revision 1.3  2001/12/08 23:23:56  peter
 * 'spara/fixar/mm'
 *
 * Revision 1.2  2001/08/06 12:16:35  peter
 * MegaMerge (se strandy_test-grenen för diffar...)
 *
 * Revision 1.1.4.1  2001/08/04 20:37:07  peter
 * flyttad hud, vapenknôk..
 *
 * Revision 1.1  2001/05/10 11:40:22  macke
 * häpp
 *
 */

#include "hw/compat.h"

#include "object/controls.h"
#include "misc/utility.h"
#include "misc/parse.h"

#include <iostream>

namespace reaper
{
namespace object
{
namespace controls
{

void Ship::select_next_missile()
{
	misc::enum_math(1, current_missile, Number_MissileTypes);
}

void Ship::select_prev_missile()
{
	misc::enum_math(-1, current_missile, Number_MissileTypes);
}

void Ship::dump(std::ostream& os) const
{
	misc::dumper(os)("pitch", pitch)
			("yaw", yaw)
			("thrust", thrust)
//			("fire", bool(fire))
//			("missile_fire", bool(missile_fire))
			("afterburner", afterburner);
			
}

}
}
}
