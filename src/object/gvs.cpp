

#include "hw/compat.h"

#include <string>

#include "object/base.h"
#include "object/hull.h"
#include "res/config.h"
#include "object/weapon.h"
#include "object/collide.h"
#include "object/current_data.h"
#include "object/controls.h"
#include "object/renderer.h"
#include "object/ai.h"
#include "object/phys.h"
#include "object/phys_data.h"
#include "main/types_param.h"
#include "main/types_io.h"
#include "object/base_data.h"

namespace reaper {
namespace object {

////////////////////////////////////////////////////////////////////////////////////////////////

class GroundShip : public DynamicBase
{
public:
	ShipInfo phys_data;	
protected:
	SillyData data;
        hull::Shielded hull;		
	current_data::GroundShip current;
	
	controls::GroundShip ctrl;
	phys::GroundShip phys;
	//sound::Turret snd;
        phys::ObjectAccessor phys_access;

public:	
	GroundShip(MkInfo);

	void collide(const CollisionInfo & cI);
        void gen_sound(reaper::sound::Interface& si)	{ }//snd.render(si); }

              phys::ObjectAccessor& get_physics()       { return phys_access; }
	const phys::ObjectAccessor& get_physics() const { return phys_access; }

	void del_waypoint(Point&) { }
};

class GroundOrdinary
	:public GroundShip
{
	ai::GVOrdinary ai;
	renderer::Ship rd;
public:
	GroundOrdinary(MkInfo);

	void think(void)                   { ai.think(); }
	void add_waypoint(const Point& p)  { ai.add_waypoint(p); }
	void erase_waypoints(void)	   { ai.erase_waypoints(); }
	void receive(const reaper::ai::Message& msg)  { ai.receive(msg); } 
	const gfx::RenderInfo* render(bool effects) const { return rd.render(effects); }
	void simulate(double dt);  
};

class GroundContainer
	:public GroundShip
{
	ai::GVContainer ai;
	renderer::Ship rd;
public:
	GroundContainer(MkInfo);

	void think(void)                  { ai.think(); }
	void add_waypoint(const Point& p) { ai.add_waypoint(p); }
	void erase_waypoints(void)	  { ai.erase_waypoints(); }
	void receive(const reaper::ai::Message& msg)  { ai.receive(msg); }
	const gfx::RenderInfo* render(bool effects) const { return rd.render(effects); }
	void simulate(double dt);  
};

class GroundTurret
 : public GroundShip
{
	const TurretInfo& phys_turret_data;
public:
	ObjTracker trk;
	MtxParams ps;
	controls::Turret ctrl_turret;	
	current_data::Turret current_turret;
	gfx::RenderInfo ri;

//	weapon::Turret weap_turret;
	weapon::Weapon* weap_turret;
	
	ai::GVTurret ai;
	phys::Turret phys_turret;	
        
//	sound::Turret snd_turret;

public:
	GroundTurret(MkInfo);

	void simulate(double dt);
	const gfx::RenderInfo* render(bool effects) const { return &ri; }
	void gen_sound(reaper::sound::Interface& si)  { } //snd_turret.render(si); }
	void think()				      { ai.think(); }
	void add_waypoint(const Point& p)             { ai.add_waypoint(p); }
	void erase_waypoints(void)	              { ai.erase_waypoints(); }
	void receive(const reaper::ai::Message& msg)  { ai.receive(msg); }
};

GroundShip::GroundShip(MkInfo mk) :
	DynamicBase(data),
	phys_data(mk.info),
	data(mk.mtx, mk.cid, mk.name), 
	hull(data, phys_data.health, phys_data.shield),
	phys(data, ctrl, current, phys_data, dyn_data, sim_time),
	phys_access(data, dyn_data, phys_data, get_radius())
{
}

GroundOrdinary::GroundOrdinary(MkInfo mk) :
	GroundShip(mk),
	ai(data, dyn_data.velocity, ctrl),
	rd(mk.name, get_mtx(), get_radius(), hull, 0, Point(0, .2, .2))
{
	add_waypoints(this, mk.cdata);
}

GroundContainer::GroundContainer(MkInfo mk) :
	GroundShip(mk),
	ai(data, dyn_data.velocity, ctrl),
	rd(mk.name, get_mtx(), get_radius(), hull, 0, Point(0, .2, .2))
{
	add_waypoints(this, mk.cdata);
}

GroundTurret::GroundTurret(MkInfo mk) :
	GroundShip(mk),
	phys_turret_data(mk.info),
	trk(get_mtx(), sim_time, get_id()),
	ps(2), 
	current_turret(0.0, 0.0), 
	ri(mk.name, get_mtx(), ps, false),
//	weap_turret(data.m, weap_data, mesh_id, ps, sim_time,"ground-turret_ball_barrel"), 
	weap_turret(weapon::mk_turret(mk, ri)),
	ai(data, dyn_data.velocity, ctrl, ctrl_turret, current_turret), 
	phys_turret(ctrl_turret, ps, current_turret, phys_turret_data, sim_time)
{
	add_waypoints(this, mk.cdata);
//	debug::std_debug << "gt init: " << mtx.get_pos() << " set: " << trk.mtx.get_pos() << '\n';
}

void GroundShip::collide(const CollisionInfo & ci) 
{
	air_damage(hull.damage(ci),ci,get_pos(),dyn_data.velocity);
}
        void GroundOrdinary::simulate(double dt)
        {
//		printf("pos: %f %f %f\n", get_pos().x, get_pos().y, get_pos().z);
                phys.simulate(dt);
		hull.regenerate(dt*phys_data.shield_recharge);
		rd.simulate(dt);
        }

	void GroundContainer::simulate(double dt)
        {
                phys.simulate(dt);
		hull.regenerate(dt*phys_data.shield_recharge);
		rd.simulate(dt);
        }

	void GroundTurret::simulate(double dt)
	{	
		phys.simulate(dt);
		double sim = get_sim_time();
		phys_turret.simulate(dt);
		set_sim_time(sim);
		hull.regenerate(dt*phys_data.shield_recharge);
	}



namespace {

SillyBase* ground_ordinary(MkInfo mk)  { return new GroundOrdinary(mk); }
SillyBase* ground_container(MkInfo mk) { return new GroundContainer(mk); }
SillyBase* ground_turret(MkInfo mk)    { return new GroundTurret(mk); }

struct Foo {
Foo() {
	factory::inst().register_object("ground-container", ground_container);
	factory::inst().register_object("ground-vehicle", ground_ordinary);
	factory::inst().register_object("ground-turret", ground_turret);
}
} bar;

} // end anonymous namespace


void gvs() { }

}
}

