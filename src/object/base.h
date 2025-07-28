#ifndef REAPER_OBJECT_BASE_H
#define REAPER_OBJECT_BASE_H

#include "main/types.h"
#include "main/enums.h"
#include "object/baseptr.h"

#include <memory>
#include <iosfwd>

namespace reaper {

namespace gfx { class RenderInfo; }
namespace sound { class Interface; }
namespace ai    { struct Message; }

namespace object {
namespace phys { class ObjectAccessor; }

namespace event { class Events; }



class SillyData;

class CollisionInfo;

class EventDispatcher;

/// Immovable, inactive simple objects (trees, ammoboxes)
class SillyBase {
protected:
	SillyData& data_r;
	SillyBase(SillyData& d);
	virtual void int_dump(std::ostream&) const { };
public:
	/** Callbacks for various events occuring, other systems may hang on
	 *  eventhandlers here, see OnDead in game/object_mgr.cpp for example
	 *  usage.  Exerimental. Will be replaced by something better.
	 */
	event::Events& events();

	/** Globally unique identifier for object. */
	ID get_id() const; 
	CompanyID get_company() const;

	void kill();
	bool is_dead() const; 

	Point get_pos() const; 
	float get_radius() const; 

	const Matrix& get_mtx() const;
//	      Matrix& get_mtx();
//	void set_mtx(const Matrix &m);


	virtual const gfx::RenderInfo* render(bool effects) const = 0;

	virtual void gen_sound() { };

	virtual void collide(const CollisionInfo&) = 0;

	virtual ~SillyBase();

	void dump(std::ostream&) const;
};

/// Immovable, active objects (gun turrets)
class StaticBase : public SillyBase {
protected:
	double sim_time;
	StaticBase(SillyData&);
public:
	double get_sim_time() const;
	void set_sim_time(double time);

	///////////////////////////////////////////////////////////
	virtual void think() = 0;
	virtual void simulate(double dt) = 0;
	virtual void receive(const ::reaper::ai::Message &) = 0;
};


struct DynamicData {
	Vector velocity;
	Vector rot_velocity;

	DynamicData();
};



/// Movable, active objects (ships, missiles)
class DynamicBase : public StaticBase {
protected:
	DynamicData dyn_data;
	DynamicBase(SillyData&);
public:
	const Vector& get_velocity() const;
	const Vector& get_rot_velocity() const;

	///////////////////////////////////////////////////////////
	virtual void add_waypoint(const Point& p) = 0;
	virtual void del_waypoint(Point& p) = 0;
	virtual       phys::ObjectAccessor& get_physics() = 0;
	virtual const phys::ObjectAccessor& get_physics() const = 0;
};

/// Player controllable objects (with a hud)
class PlayerBase : public DynamicBase {
protected:
	PlayerBase(SillyData&);
public:
	enum HUD { Internal, External };
	virtual void render_hud(HUD) = 0;

}; 

/// Movable, inactive objects (lazer shots, cannon balls etc)
class ShotBase : public SillyBase {
protected:
	double sim_time;
	const float lifespan;
	Vector velocity;
	ID parent;
	ShotBase(SillyData& d, double st, float ls, const Vector &v, ID p);
public:
	double get_sim_time() const;
	void set_sim_time(double time);
	float get_lifespan() const;
	const Vector& get_velocity() const;

	///////////////////////////////////////////////////////////
	virtual void simulate(double dt) = 0;
	virtual float get_damage() const = 0;
	ID get_parent() { return parent; }
};

struct ObjTracker
{
	const Matrix& mtx;
	double& sim_time;
	ID id;
	ObjTracker(const Matrix& m, double& s, ID i)
	 : mtx(m), sim_time(s), id(i)
	{ }
};


}
}

#endif

/*
 * $Author: pstrand $
 * $Date: 2002/04/09 03:36:31 $
 * $Log: base.h,v $
 * Revision 1.40  2002/04/09 03:36:31  pstrand
 * eff.ptr koll, shot-parent-id, quadtree-update-update
 *
 * Revision 1.39  2002/04/06 20:13:31  pstrand
 * the matrix encapsulated one more level, use SillyData::set_mtx to change (or objectaccessor)
 *
 * Revision 1.38  2002/02/08 11:27:55  macke
 * rendering reorg and silhouette shadows!
 *
 * Revision 1.37  2002/01/23 01:00:01  peter
 * soundfixes again
 *
 * Revision 1.36  2002/01/22 23:44:07  peter
 * reversed last two revs
 *
 * Revision 1.34  2002/01/17 05:05:57  peter
 * *** empty log message ***
 *
 * Revision 1.33  2002/01/07 14:00:32  peter
 * resurs och ljudmeck
 *
 * Revision 1.32  2001/12/13 17:03:30  peter
 * småfixar...
 *
 * Revision 1.31  2001/12/08 23:23:55  peter
 * 'spara/fixar/mm'
 *
 * Revision 1.30  2001/12/06 20:00:06  peter
 * experimental antifunctional dump
 *
 * Revision 1.29  2001/12/03 18:06:17  peter
 * object/base.h-meck...
 *
 * Revision 1.28  2001/12/02 17:40:48  peter
 * objmgrmeck
 *
 * Revision 1.27  2001/11/26 10:46:06  peter
 * kompilatorgnäll... ;)
 *
 * Revision 1.26  2001/11/26 02:20:15  peter
 * ljudfixar (smartptr), andra mindre fixar..
 *
 * Revision 1.25  2001/11/21 00:54:19  peter
 * death...
 *
 * Revision 1.24  2001/09/10 21:55:26  peter
 * no message
 *
 * Revision 1.23  2001/08/27 12:55:27  peter
 * objektmeck...
 *
 * Revision 1.22  2001/08/20 17:09:57  peter
 * obj.ptr & fix...
 *
 * Revision 1.21  2001/08/06 12:16:35  peter
 * MegaMerge (se strandy_test-grenen för diffar...)
 *
 * Revision 1.20.2.7  2001/08/05 14:01:29  peter
 * objektmeck...
 *
 * Revision 1.20.2.6  2001/08/05 00:11:10  peter
 * döda...
 *
 * Revision 1.20.2.5  2001/08/04 22:45:24  peter
 * impl.h is no more..
 *
 * Revision 1.20.2.4  2001/08/04 20:37:07  peter
 * flyttad hud, vapenknôk..
 *
 * Revision 1.20.2.3  2001/08/03 13:44:10  peter
 * pragma once obsolete...
 *
 * Revision 1.20.2.2  2001/08/03 13:37:11  peter
 * new weapon...
 *
 * Revision 1.20.2.1  2001/08/02 18:20:49  peter
 * *** empty log message ***
 *
 * Revision 1.20  2001/07/30 23:43:25  macke
 * Häpp, då var det kört.
 *
 * Revision 1.19  2001/07/23 23:48:10  macke
 * Slimmad grafikhantering samt lite namnbyten
 *
 * Revision 1.18  2001/07/06 01:47:30  macke
 * Refptrfix/headerfilsstäd/objekt-skapande/mm
 *
 * Revision 1.17  2001/05/14 20:00:55  macke
 * bugfix och rök på missiler..
 *
 * Revision 1.16  2001/05/14 11:22:40  picon
 * Fullösningsförberedelse
 *
 *
 */


