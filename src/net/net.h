#ifndef REAPER_NET_NET_H
#define REAPER_NET_NET_H

#include <iostream>
#include <set>
#include <map>
#include <deque>

#include "main/types.h"
#include "hw/reltime.h"
#include "object/base.h"

/// Reaper!
namespace reaper
{
/// Networking.
namespace net
{





typedef object::DynamicPtr NetObjPtr;

typedef hw::time::RelTime Timestamp;


typedef std::map<object::ID, NetObjPtr> IntObjStates;
typedef IntObjStates::iterator int_state_iter;


struct ObjState
{
	object::ID id;
	hw::time::RelTime sync;
	Matrix mat;
	Vector vel;
	Vector rot_vel;
	bool dead;

	ObjState() : id(0), sync(0) { }
	ObjState(int i, hw::time::RelTime rt,
		 const Matrix& m, const Vector& v, const Vector& rv, bool d = false)
	 : id(i), sync(rt), mat(m), vel(v), rot_vel(rv), dead(d)
	{ }

	bool operator<(const ObjState& os) const {
		return id < os.id;
	}
};

std::ostream& operator<<(std::ostream&, const ObjState&);
std::istream& operator>>(std::istream&, ObjState&);

typedef std::set<ObjState> ObjStates;
typedef std::deque<ObjState> ObjQueue;
typedef ObjStates::iterator state_iter;
typedef std::map<object::ID, ObjState> ObjStateMap;
typedef ObjStateMap::const_iterator statemap_citer;
typedef ObjStateMap::iterator statemap_iter;


typedef int GameState;
const int init = 0;
const int active = 1;
const int startup = 2;
const int running = 4;


}
}

#endif


/*
 * $Author: pstrand $
 * $Date: 2002/09/20 22:09:26 $
 * $Log: net.h,v $
 * Revision 1.31  2002/09/20 22:09:26  pstrand
 * *** empty log message ***
 *
 * Revision 1.30  2002/08/12 11:38:43  pstrand
 * *** empty log message ***
 *
 * Revision 1.29  2002/04/16 19:59:30  pstrand
 * send only units
 *
 * Revision 1.28  2002/03/24 17:37:47  pstrand
 * no message
 *
 * Revision 1.27  2002/03/19 20:39:11  pstrand
 * reorg&improvement..
 *
 * Revision 1.26  2002/03/18 13:33:04  pstrand
 * network fixes
 *
 * Revision 1.25  2002/03/11 10:50:49  pstrand
 * hw/hwtime.h -> reltime.h&abstime.h, and some more..
 *
 * Revision 1.24  2001/08/20 17:08:23  peter
 * obj.ptr
 *
 * Revision 1.23  2001/08/06 12:16:34  peter
 * MegaMerge (se strandy_test-grenen för diffar...)
 *
 * Revision 1.22.2.1  2001/08/03 13:44:09  peter
 * pragma once obsolete...
 *
 * Revision 1.22  2001/07/30 23:43:24  macke
 * Häpp, då var det kört.
 *
 * Revision 1.21  2001/07/06 01:47:29  macke
 * Refptrfix/headerfilsstäd/objekt-skapande/mm
 *
 * Revision 1.20  2001/05/14 23:43:41  peter
 * *** empty log message ***
 *
 * Revision 1.19  2001/05/14 18:53:04  peter
 * *** empty log message ***
 *
 * Revision 1.18  2001/05/12 17:40:53  peter
 * *** empty log message ***
 *
 */

