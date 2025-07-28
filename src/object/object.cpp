#include "hw/compat.h"
#include "object/object.h"
#include "object/object_impl.h"
#include "object/projectile.h"

namespace reaper {
namespace object {

int& obj_count() {
	static int count = 0;
	return count;
}

void obj_count_incr()
{
	++obj_count();
}	

void obj_count_decr()
{
	--obj_count();
}

int get_obj_count()
{
	return obj_count();
}


ShotBase* laser(const Matrix &m, CompanyID c, double sim, ID id)
{
	return new Laser(m, c, sim, id);
}

ShotBase* light_missile(const Matrix &m, CompanyID c, double sim, ID id)
{
	return new Missile("light-missile", m, c, sim, id);
}

ShotBase* heavy_missile(const Matrix &m, CompanyID c, double sim, ID id)
{
	return new Missile("heavy-missile", m, c, sim, id);
}





}
}

