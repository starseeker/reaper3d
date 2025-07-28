
#include "hw/compat.h"

#include "hw/debug.h"

#include <string>

#include "object/base.h"
#include "object/base_data.h"
#include "main/types_io.h"

#include "object/event.h"

#include "world/world.h"

#include "misc/parse.h"
#include "main/types_io.h"

#include "gfx/managers.h"

namespace reaper {
namespace object {

namespace { debug::DebugOutput derr("object::base"); }

void obj_count_incr();
void obj_count_decr();
int get_obj_count();


SillyData::SillyData(const Matrix &mtx, CompanyID c, const std::string& n, ID i)
: id(i == -1 ? world::alloc_id() : i), name(n), dead(false), 
  radius(gfx::MeshMgr::get_ref()->get_radius(name)), 
  company(c), m(mtx), events(new event::Events())
{
}

SillyData::SillyData(const Matrix &mtx, CompanyID c, const std::string& n, float r, ID i)
: id(i == -1 ? world::alloc_id() : i), name(n), dead(false), 
  radius(r), company(c), m(mtx), events(new event::Events())
{
}

SillyData::~SillyData()
{
	delete events;
}

ID SillyData::get_id() const
{
	return id;
}

CompanyID SillyData::get_company() const
{
	return company;
}

void SillyData::kill()
{
	// multiple kills are ignored, is this right?
	if (!dead) {
		dead = true;
		events->death.notify().send();
		events->death.clear();
	} else {
		// It should never happen anymore, so we print a warning -peter
		derr << "Warning: Multiple kills!\n";
	}
}

bool SillyData::is_dead() const
{
	return dead;
}

Point SillyData::get_pos() const
{
	return m.pos();
}

float SillyData::get_radius() const
{
	return radius;
}

const Matrix& SillyData::get_mtx() const
{
	return m;
}

void SillyData::set_mtx(const Matrix& mtx)
{
	m = mtx;
}

const std::string& SillyData::get_name() const
{
	return name;
}

SillyBase::SillyBase(SillyData& d)
 : data_r(d)
{
	obj_count_incr();
}

SillyBase::~SillyBase()
{
	obj_count_decr();
}

ID SillyBase::get_id() const
{
	return data_r.id;
}

bool SillyBase::is_dead() const
{
	return data_r.dead;
}

Point SillyBase::get_pos() const
{
	return data_r.m.pos();
}

float SillyBase::get_radius() const
{
	return data_r.radius;
}

CompanyID SillyBase::get_company() const
{
	return data_r.company;
}

const Matrix& SillyBase::get_mtx() const
{
	return data_r.get_mtx();
}

event::Events& SillyBase::events()
{
	return *data_r.events;
}

void SillyBase::kill()
{
	data_r.kill();
}

using misc::mk_config_line;
void SillyBase::dump(std::ostream& os) const
{
	misc::dumper(os)("object", data_r.get_name())
			("id", data_r.get_id())
			("company", data_r.get_company())
			("matrix", data_r.m);
	int_dump(os);
}


StaticBase::StaticBase(SillyData& d)
 : SillyBase(d), sim_time(0)
{ }

double StaticBase::get_sim_time() const
{
	return sim_time;
}

void StaticBase::set_sim_time(double time)
{
	sim_time = time;
}

DynamicData::DynamicData()
 : velocity(0,0,0), rot_velocity(0,0,0)
{ }


DynamicBase::DynamicBase(SillyData& d)
 : StaticBase(d)
{ }

const Vector& DynamicBase::get_velocity() const
{
	return dyn_data.velocity;
}

const Vector& DynamicBase::get_rot_velocity() const
{
	return dyn_data.rot_velocity;
}

PlayerBase::PlayerBase(SillyData& d)
 : DynamicBase(d)
{ }

ShotBase::ShotBase(SillyData& d, double st, float ls, const Vector &v, ID p)
 : SillyBase(d), sim_time(st), lifespan(ls), velocity(v), parent(p)
{ }

double ShotBase::get_sim_time() const
{
	return sim_time;
}

void ShotBase::set_sim_time(double time)
{
	sim_time = time;
}

float ShotBase::get_lifespan() const
{
	return lifespan;
}

const Vector& ShotBase::get_velocity() const
{
	return velocity;
}


}
}


