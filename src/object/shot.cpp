
#include "hw/compat.h"

#include "main/types.h"
#include "object/base.h"
#include "object/factory.h"
#include "object/projectile.h"

namespace reaper {
namespace object {



factory::ObjBase* laser(MkInfo mk)
{
	return new Laser(mk.mtx, mk.cid, 0, -1);
}

factory::ObjBase* light_missile(MkInfo mk)
{
	return new Missile("light-missile", mk.mtx, mk.cid, 0, -1);
}

factory::ObjBase* heavy_missile(MkInfo mk)
{
	return new Missile("heavy-missile", mk.mtx, mk.cid, 0, -1);
}


struct Foo {
	Foo() {
		factory::inst().register_object("laser", laser);
		factory::inst().register_object("light-missile", light_missile);
		factory::inst().register_object("heavy-missile", heavy_missile);
	}
} bar;

void shot() { }

}
}

