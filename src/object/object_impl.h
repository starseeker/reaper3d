
/* $Id: object_impl.h,v 1.21 2002/04/06 20:13:31 pstrand Exp $ */

#ifndef REAPER_OBJECT_OBJECT_IMPL_H
#define REAPER_OBJECT_OBJECT_IMPL_H

#include "object/base.h"
#include "object/base_data.h"
#include "object/controls.h"
#include "object/hull.h"
#include "object/renderer.h"
#include "object/ai.h"
#include "object/player_ai.h"
#include "object/phys.h"
#include "object/current_data.h"
#include "object/factory.h"

#include "main/types_param.h"


#include <vector>

namespace reaper {
namespace object {


using ::reaper::object::phys::ObjectAccessor;


template<class Base>
class ObjImpl : public Base
{
protected:
	SillyData data;
public:
	/* Note, the SillyData chunk is not initialised before
	   the base class initialisation, however this should 
	   not cause any trouble, as it is now. If needed, use an
	   auxillary (sillydata-initalisation) class and inherit before Base.
	 */
	ObjImpl(const Matrix& mtx, CompanyID c, const std::string& n, ID i = -1)
	 : Base(data), data(mtx, c, n, i)
	{ }
	ObjImpl(const MkInfo& mk)
	 : Base(data), data(mk.mtx, mk.cid, mk.name, mk.id)
	{ }
};


class GenShip : public ObjImpl<PlayerBase>
{
public:
	ShipInfo phys_data;
protected:
	ObjTracker trk;
	current_data::Ship current;
        hull::Shielded hull;
	phys::Ship phys;
	renderer::Ship rd;
	controls::Ship ctrl;
	sound::Ship* snd;

	weapon::Weapon* laser;
	weapon::Missiles* missiles;

	phys::ObjectAccessor phys_access;

	void int_dump(std::ostream&) const;
public:
	explicit GenShip(MkInfo);
	~GenShip();

	void collide(const CollisionInfo& cI);
	const gfx::RenderInfo* render(bool effects) const { return rd.render(effects); }
	void gen_sound();
	void simulate(double dt);

	void set_pos(const Point &p) { data.set_mtx(Matrix(p)); }

	      phys::ObjectAccessor& get_physics()       { return phys_access; }
	const phys::ObjectAccessor& get_physics() const { return phys_access; }
	
	void render_hud(PlayerBase::HUD h);
	
};

class SillyImpl
 : public ObjImpl<SillyBase>
{
protected:
	gfx::RenderInfo ri;
	hull::Standard hull;
	void int_dump(std::ostream&) const;
public:
	explicit SillyImpl(const MkInfo& mk);
	const gfx::RenderInfo* render(bool effects) const { return &ri; }
	void collide(const CollisionInfo&);
};

////////////////////////////////////////////////////////////////////////////////////////////////

}
}
#endif

