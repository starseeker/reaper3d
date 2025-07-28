
#include "hw/compat.h"

#include "object/object_impl.h"
#include "gfx/gfx_types.h"
#include "misc/monitor.h"
#include "misc/stlhelper.h"
#include "misc/parse.h"
#include "object/weapon.h"
#include "object/sound.h"
#include "object/collide.h"
#include "object/event.h"

#include "hw/event.h"
#include "hw/debug.h"
#include "main/types_io.h"
#include "res/config.h"

#include "gfx/managers.h"

namespace reaper {
namespace object {

template<class B, class C>
void set_weapon(misc::Switch* sw, B& b, C c)
{
	sw->set(misc::mk_cmd(b, &weapon::Weapon::fire, c));
}

class notify_death
{
	renderer::Ship &r;
	sound::Ship* s;
public:
	notify_death(renderer::Ship &rr, sound::Ship* ss) : r(rr), s(ss) {}
	void operator()() {
		r.kill();
		s->kill();
	}
};

GenShip::GenShip(MkInfo mk) :
	ObjImpl<PlayerBase>(mk),
	phys_data(mk.info),
	trk(get_mtx(), sim_time, get_id()), 
	hull(data, phys_data.health, phys_data.shield),
	phys(data, ctrl, current, phys_data, dyn_data, sim_time),
	rd(mk.name, get_mtx(), get_radius(), hull, 1, phys_data.engine_pos), // trail size = 1
	snd(sound::create_ship(mk.info["real_name"], get_mtx(), dyn_data.velocity)), 
	laser(weapon::mk_laser(mk)),
	missiles(weapon::mk_missiles(mk)),
	phys_access(data, dyn_data, phys_data, get_radius())
{
	set_weapon(&ctrl.fire, laser, trk);
	set_weapon(&ctrl.missile_fire, missiles, trk);
	ctrl.current_missile.onchange(misc::mk_cmd(missiles, &weapon::Missiles::select));
	current.eng.thrust.onchange(misc::mk_cmd(snd, &sound::Ship::engine_pitch));
	events().death.add_listener(misc::mk_cmd<void, notify_death>(notify_death(rd, snd)));
}

GenShip::~GenShip()
{
	delete laser;
	delete missiles;
	delete snd;
}

void GenShip::gen_sound()
{
	if (snd) {
		snd->set_location(get_mtx());
		snd->set_velocity(dyn_data.velocity);
	}
}

void GenShip::render_hud(HUD h)
{
	if (snd)
		snd->inside_view(h == Internal);
	rd.render_hud(h, hull, phys, phys_data, ctrl.current_missile, missiles);
}


void GenShip::collide(const CollisionInfo & ci) 
{
	hull::DamageStatus ds = hull.damage(ci);
	air_damage(ds,ci,get_pos(),dyn_data.velocity);
	if(ds == hull::Shield) {
		rd.shield_hit(ci);
	}
}

void GenShip::simulate(double dt) 
{
	phys.simulate(dt);
	hull.regenerate(dt*phys_data.shield_recharge);
	rd.simulate(dt);
}

void GenShip::int_dump(std::ostream& os) const
{
	hull.dump(os);
	ctrl.dump(os);
	phys_data.dump(os);
}

SillyImpl::SillyImpl(const MkInfo& mk)
 : ObjImpl<SillyBase>(mk),
   ri(mk.name, get_mtx(), false),
   hull(data, mk.info["hull_strength"])
{
}

void SillyImpl::collide(const CollisionInfo& ci)
{
	damage(hull.damage(ci),ci,get_pos());
}

void SillyImpl::int_dump(std::ostream& os) const
{
	hull.dump(os);
}

}
}

