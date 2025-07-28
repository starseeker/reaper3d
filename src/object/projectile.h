
#ifndef REAPER_OBJECT_PROJECTILE_H
#define REAPER_OBJECT_PROJECTILE_H

#include "object/hull.h"
#include "object/phys_data.h"
#include "object/forward.h"
#include "object/phys.h"
#include "object/renderer.h"
#include "object/factory.h"
#include "object/sound.h"
#include "object/collide.h"

namespace reaper {
namespace object {

class ProjectileBaseData
{
protected:
	SillyData data;
	ShotInfo phys_data;
	ProjectileBaseData(const std::string& name, const Matrix& mtx, CompanyID c);
};

class ProjectileBase
 : protected ProjectileBaseData
 , public ShotBase
{
	phys::Laser phys;
	float death_time;
	void move();
protected:
	sound::Projectile* sound;
	hull::Shielded hull;  //Dummy hull

public:
	ProjectileBase(const std::string& name, const Matrix &mtx, CompanyID c, double sim, ID p);

	~ProjectileBase();

	void receive(const reaper::ai::Message &) { }
	float get_damage() const		  { return phys_data.damage; }
	void simulate(double dt);
	void gen_sound(reaper::sound::Interface& si) { move(); }
};

class Laser
 : public ProjectileBase
{
	renderer::Laser rd;

	void on_kill();
public:
	Laser(const Matrix &mtx, CompanyID c, double sim, ID p);
	void collide(const CollisionInfo &);
	const gfx::RenderInfo* render(bool effects) const { return rd.render(effects); }
};

class Missile : public ProjectileBase
{
	renderer::Missile rd;

	void on_kill();
public:
	Missile(const std::string& id, const Matrix &mtx, CompanyID c, double sim, ID p);
	void collide(const CollisionInfo &);
	const gfx::RenderInfo* render(bool effects) const { return rd.render(effects); }
};

}
}
#endif

