
/* $Id: hull.h,v 1.18 2002/02/11 12:03:15 peter Exp $ */


#ifndef REAPER_OBJECT_HULL_H
#define REAPER_OBJECT_HULL_H

#include <iostream>

#include "object/base.h"
#include "misc/parse.h"

namespace reaper {
namespace object {
namespace hull {

enum DamageStatus {
	Absorbed, ///< Hull takes no damage from this collision
	Shield,   ///< The shield takes all damage
	Hull,     ///< The hull is injured
	Fatality  ///< Hull is destroyed
};

class Standard
{
	SillyData &data;
	float health;
public:
	Standard(SillyData &d, float h) : data(d), health(h) {}

	DamageStatus damage(const CollisionInfo & ci);		
	float get_health() const { return health; }

	void dump(std::ostream& os) const
	{
		misc::dumper(os)("health", health);
	}
};

class Shielded : public Standard
{
	float shield;
	bool shield_hit;
	const float shield_start;
public:
	Shielded(SillyData &d, float h, float s)
	 : Standard(d,h), shield(s), 
	   shield_hit(false),shield_start(s)  
	{ }

	DamageStatus damage(const CollisionInfo & ci);
	void regenerate(float ds);
	/// Resets shield hit status if set and returns previous state
	bool read_shield_hit();
	float get_shield() const { return shield; }

	void dump(std::ostream& os) const
	{
		Standard::dump(os);
		misc::dumper(os)("sheld", shield)
				("shield_start", shield_start);
	}
};
}
}
}
#endif


