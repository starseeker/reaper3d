
#include "hw/compat.h"

#include "main/types_param.h"
#include "object/base.h"
#include "object/factory.h"
#include "object/hull.h"
#include "object/ai.h"
#include "object/renderer.h"
#include "res/config.h"
#include "main/types_io.h"
#include "hw/event.h"
#include "object/weapon.h"
#include "object/collide.h"
#include "object/controls.h"
#include "object/current_data.h"
#include "object/phys.h"
#include "gfx/managers.h"

#include "object/object_impl.h"

namespace reaper {
namespace object {


class Turret : public ObjImpl<StaticBase>
{
public:
	const TurretInfo phys_data;
protected:
	ObjTracker trk;
	hull::Standard hull;
	MtxParams ps;

	current_data::Turret current;
	controls::Turret ctrl;
	gfx::RenderInfo ri;
	weapon::Weapon* weap;

	ai::Turret ai;
	phys::Turret phys;	        

public: 
	//        static const reaper::object::WeaponInfo weap_data;

	Turret(MkInfo mk) :
	   ObjImpl<StaticBase>(mk),
	   phys_data(mk.info),
	   trk(get_mtx(), sim_time, get_id()),
	   hull(data, phys_data.health), 
	   ps(2), 
	   current(0.0, 0.0), 
	   ri(mk.name, get_mtx(), ps, false),
	   weap(weapon::mk_turret(mk, ri)),
	   ai(data, ctrl, current, gfx::MeshMgr::get_ref()->get_matrix(ri, "turret_ball").pos()), 
	   phys(ctrl, ps, current, phys_data, sim_time)
	{
		ctrl.fire.set(misc::mk_cmd(weap, &weapon::Weapon::fire, trk));
//		printf("created turret %x\n", this);
	}
	~Turret()
	{
//		printf("delete turret %x\n", this);
		delete weap;
	}

	void collide(const CollisionInfo & ci)
	{
		damage(hull.damage(ci), ci, get_pos());
	}
	void think()                                   { ai.think(); }
	void receive(const reaper::ai::Message& msg)   {}
	const gfx::RenderInfo* render(bool effects) const { return &ri; }
	void simulate(double dt)                       { phys.simulate(dt); }
	void gen_sound(reaper::sound::Interface& si)   { }
};



void turret() { }
SillyBase* turret(MkInfo mk) { return new Turret(mk); }
namespace {
struct Foo {
	Foo() { factory::inst().register_object("turret", turret); }
} foo;
}
}
}
