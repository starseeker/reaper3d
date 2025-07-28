
#include "hw/compat.h"

#include "hw/debug.h"

#include "object/event.h"
#include "misc/command.h"
#include "object/projectile.h"
#include "misc/free.h"
#include "snd/sound_system.h"

namespace reaper {
namespace object {

namespace { debug::DebugOutput derr("projectile"); }

ProjectileBaseData::ProjectileBaseData(const std::string& name, 
			const Matrix& mtx, CompanyID c)
  : data(mtx, c, name, 0.2f),
    phys_data(factory::inst().info(name))
{ }

ProjectileBase::ProjectileBase(const std::string& name, const Matrix &mtx, CompanyID c, double sim, ID p) :
	ProjectileBaseData(name, mtx, c),
	ShotBase(data, sim, phys_data.lifespan, mtx * Vector(0, 0, -phys_data.speed), p), 
	phys(data, get_mtx()*Vector(0, 0, -phys_data.speed), phys_data, sim_time,sim_time+phys_data.lifespan),
	death_time(sim_time+phys_data.lifespan),
	sound(0),
	hull(data, 0.0, 0.0)  //A dummy hull with zero life
{
}

ProjectileBase::~ProjectileBase()
{
}

void ProjectileBase::move()
{
	if (sound) {
		sound->set_location(data.get_mtx());
		sound->set_velocity(velocity);
	}
}

Laser::Laser(const Matrix &mtx, CompanyID c, double sim, ID p) : 
	ProjectileBase("laser", mtx, c, sim, p),
	rd(phys_data, get_mtx())
{
	sound = sound::create_proj(phys_data.sound, mtx, velocity);
	events().death.add_listener(misc::mk_cmd(this, &Laser::on_kill));
}


Missile::Missile(const std::string& id, const Matrix &mtx, CompanyID c, double sim, ID p) : 
	ProjectileBase(id, mtx, c, sim, p),
	rd(id, phys_data, get_mtx(), get_radius())
{
//	sound = sound::create_proj("missile_fly", mtx, velocity);
	if (reaper::sound::Manager::get_ref().valid())
		reaper::sound::Manager::get_ref()->play(phys_data.sound, get_pos());

	events().death.add_listener(misc::mk_cmd(this, &Missile::on_kill));
}

void Laser::on_kill()
{
	hull_hit(get_pos(), phys_data.exp_intensity);
	gfx::static_light(get_pos(), phys_data.color, phys_data.light_radius,2).insert_release();
	misc::zero_delete(sound);
}

void Laser::collide(const CollisionInfo&)
{
	kill();
}

void Missile::on_kill()
{
	hull_hit(get_pos(), phys_data.exp_intensity);
	gfx::static_light(get_pos() + Vector(0,5,0), phys_data.color,
			  phys_data.light_radius, .3).insert_release();
	rd.kill();
	misc::zero_delete(sound);
}

void Missile::collide(const CollisionInfo&)
{
	kill();
}

void ProjectileBase::simulate(double dt)
{
	phys.simulate(dt);
}

}
}

