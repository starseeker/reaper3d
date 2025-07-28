#ifndef REAPER_OBJECT_WEAPON_H
#define REAPER_OBJECT_WEAPON_H

#include "object/base.h"
#include "object/factory.h"
#include "object/phys_data.h"

namespace reaper {
class MtxParams; 
namespace object { 
namespace weapon {


class Weapon
{
public:
	virtual bool fire(ObjTracker) = 0;
	virtual ~Weapon() { }
};

class Missiles : public Weapon
{
	MissileType current;
public:
	Missiles(MissileType);
	void select(MissileType);
	MissileType get_current() const;
	int get_num(MissileType) const;
};


Weapon* mk_laser(const MkInfo& mk);
Missiles* mk_missiles(const MkInfo& mk);
Weapon* mk_turret(const MkInfo& mk,const gfx::RenderInfo& ri);


}
}
}


#endif

/*
 * $Author: pstrand $
 * $Date: 2002/08/12 11:39:28 $
 * $Log: weapon.h,v $
 * Revision 1.20  2002/08/12 11:39:28  pstrand
 * *** empty log message ***
 *
 * Revision 1.19  2002/02/08 11:27:59  macke
 * rendering reorg and silhouette shadows!
 *
 * Revision 1.18  2001/11/26 10:46:06  peter
 * kompilatorgn�ll... ;)
 *
 * Revision 1.17  2001/08/27 12:55:31  peter
 * objektmeck...
 *
 * Revision 1.16  2001/08/06 12:16:41  peter
 * MegaMerge (se strandy_test-grenen f�r diffar...)
 *
 * Revision 1.15.2.7  2001/08/05 20:04:24  peter
 * vc "fix"
 *
 * Revision 1.15.2.6  2001/08/04 20:37:08  peter
 * flyttad hud, vapenkn�k..
 *
 * Revision 1.15.2.5  2001/08/04 16:14:18  peter
 * *** empty log message ***
 *
 * Revision 1.15.2.4  2001/08/03 13:44:13  peter
 * pragma once obsolete...
 *
 * Revision 1.15.2.3  2001/08/03 13:37:13  peter
 * new weapon...
 *
 */

