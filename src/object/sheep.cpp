
#include "hw/compat.h"

#include "object/base.h"
#include "object/factory.h"
#include "object/hull.h"
#include "object/object_impl.h"
#include "res/config.h"
#include "main/types_io.h"
#include "misc/utility.h"
#include "hw/debug.h"

namespace reaper {
namespace object {
namespace {
reaper::debug::DebugOutput dout("object::Sheep",0);
} // end anonymous namespace


class Sheep : public ObjImpl<DynamicBase>
{
	hull::Standard hull;		
	ObjectAccessor phys_access;
	gfx::RenderInfo ri;
	world::WorldRef wr;
public:

	Sheep(const Matrix &mtx, CompanyID c, ID i);

	void collide(const CollisionInfo& cI);
	const gfx::RenderInfo* render(bool effects) const { return &ri; }
	void simulate(double dt);		
	void think() {}                                 // dumb sheep
	void receive(const reaper::ai::Message& msg) {} // deaf sheep

	ObjectAccessor& get_physics()       { return phys_access; }
	const ObjectAccessor& get_physics() const { return phys_access; }

	virtual void add_waypoint(const Point& p) {}
	virtual void del_waypoint(Point& p) {}
};

Sheep::Sheep(const Matrix &mtx, CompanyID c, ID i)
 : ObjImpl<DynamicBase>(mtx, c, "sheep", i),
   hull(data, 1), 
   phys_access(data, dyn_data, 0.01, 20, 0.1, get_radius()), 
   ri("sheep", get_mtx(), false),
   wr(world::World::get_ref())
{
	using reaper::misc::frand;
	dyn_data.velocity = Matrix(frand()*360, Vector(0,1,0)) * Vector(3 + 3*frand(), 0, 0);	
}
void Sheep::collide(const CollisionInfo& ci)
{
	if(hull.damage(ci) == hull::Fatality) {
		gfx::blood_n_guts(get_pos(),1).insert_release();
	} else {
		gfx::blood_n_guts(get_pos(),.1).insert_release();
	}
	dyn_data.velocity *= -1;
}

void Sheep::simulate(double dt)
{
	using namespace ::reaper::world;		
	Matrix m(get_mtx());

	float dt2 = dt;
	while(dt2>.02) {
		dyn_data.velocity = Matrix(dt2*360/50,Vector(0,1,0)) * dyn_data.velocity;
		m.pos().x += dyn_data.velocity.x * dt2;
		m.pos().z += dyn_data.velocity.z * dt2;		
		dt2 -= .02;
	}
	dyn_data.velocity = Matrix(dt2*360/50,Vector(0,1,0)) * dyn_data.velocity;
	m.pos().x += dyn_data.velocity.x * dt2;
	m.pos().z += dyn_data.velocity.z * dt2;		

	m.pos().y = wr->get_altitude(Point2D(get_pos().x, get_pos().z)) + 1.6f; 

	data.set_mtx(m);
	sim_time += dt;		
}

SillyBase* sheep(MkInfo mk)
{
	return new Sheep(mk.mtx, mk.cid, mk.id);
}



namespace {
	struct Foo {
		Foo() {
			factory::inst().register_object("sheep", sheep);
		}
	} bar;
}

void sheep() { }

}
}
