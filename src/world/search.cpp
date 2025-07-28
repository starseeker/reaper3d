/*
 * $Author: pstrand $
 * $Date: 2002/04/14 09:22:37 $
 * $Log: search.cpp,v $
 * Revision 1.15  2002/04/14 09:22:37  pstrand
 * quadtree redesign and Sphere::radius -> Sphere::r
 *
 * Revision 1.14  2002/03/18 13:34:08  pstrand
 * rect search & level fix
 *
 * Revision 1.13  2001/11/29 00:53:59  peter
 * 'erase'
 *
 * Revision 1.12  2001/11/27 00:54:55  peter
 * worlditeratorer lämnar inte ifrån sig döda objekt längre..
 *
 * Revision 1.11  2001/11/10 13:57:05  peter
 * minnesfixar...
 *
 * Revision 1.10  2001/08/20 20:31:42  peter
 * no message
 *
 * Revision 1.9  2001/08/20 19:12:16  macke
 * Grin från intel..
 *
 * Revision 1.8  2001/08/20 17:11:45  peter
 * obj.ptr
 *
 * Revision 1.7  2001/08/06 12:16:46  peter
 * MegaMerge (se strandy_test-grenen för diffar...)
 *
 * Revision 1.6.2.1  2001/07/31 17:34:10  peter
 * testgren...
 *
 * Revision 1.6  2001/07/30 23:43:35  macke
 * Häpp, då var det kört.
 *
 * Revision 1.5  2001/05/14 21:58:45  niklas
 * Deallokerar inte objekt vid borttagning
 *
 * Revision 1.4  2001/05/06 09:25:33  peter
 * frustum, const.. (förebygga senilitet hos undertecknad..)
 *
 * Revision 1.3  2001/05/06 00:44:21  peter
 * erase
 *
 * Revision 1.2  2001/04/25 13:35:42  peter
 * no message
 *
 * Revision 1.1  2001/04/25 00:49:04  peter
 * omorganisering...
 *
 */

#include "hw/compat.h"

#include <map>
#include <limits>

#include "misc/smartptr.h"
#include "misc/sequence.h"
#include "world/geometry.h"
#include "world/search.h"
#include "world/world.h"
#include "object/base.h"

namespace reaper
{
namespace world
{

using misc::seq;
using misc::lower_bound;
using misc::upper_bound;



template<class C>
ftuple search_iterator<C>::get_limits()
{
	Vector tmp;
	float mid = 0;
	float rad = std::numeric_limits<float>::max();
	switch (what) {
	case F_Frustum:
			mid = frustum.pos().x + frustum.pos().z;
			rad = frustum.b_radius();
			break;
	case F_Line:
			tmp = (frustum.dir() - frustum.pos().to_vec()) / 2;
			mid = frustum.pos().x + tmp.x + frustum.pos().z + tmp.z;
			rad = length(tmp);
			break;
	case F_Cylinder:
	case F_Sphere:  mid = sphere.p.x + sphere.p.z;
			rad = sphere.r;
			break;
	case F_Rect:
			rad = (rect.ur.x - rect.ll.x)/2;
			mid = rect.ll.x + rad;
			break;
	default: break;
	}
	return std::make_pair(mid, rad);
}

template<class C> void search_iterator<C>::init()
{
	end = cont->end();
	if (cont && ! cont->empty()) {
		cur = cont->begin();
	} else {
		cont = 0;
		return;
	}
}

template<class C> void search_iterator<C>::init_find()
{
	init();
	if (!cont)
		return;
	if (what != F_All) {

		ftuple tp = get_limits();
		EL_it b = cont->begin();
		EL_it e = cont->end();
		value_type b1(tp.first - 2*tp.second);
		value_type b2(tp.first + 2*tp.second);
		cur = std::lower_bound(b, e, b1);
		end = std::upper_bound(b, e, b2);
	}
	find();
}



bool not_dead(Box<object::DynamicPtr> p)
{
	return ! p->is_dead();
}

bool not_dead(Box<object::ShotPtr> p)
{
	return ! p->is_dead();
}

template<class C>
bool not_dead(C)
{
	return true;
}




template<class C> void search_iterator<C>::find()
{
	while (cur != end) {
		if (test_next() && not_dead(*cur))
			return;
		cur++;
	}
	cont = 0;
}

template<class C> bool search_iterator<C>::test_next()
{
	if (what == F_All)
		return true;
	
	Point2D sp(sphere.p.x, sphere.p.z);
	typename C::value_type o = (*cur).get();
	Point2D os(o->get_pos().x, o->get_pos().z);
	switch (what) {
	case F_Cylinder:
		return intersect(sp, sphere.r, os, o->get_radius());
	case F_Sphere:
		return intersect(sphere.p, sphere.r, o->get_pos(), o->get_radius());
	case F_Line:
		return intersect(o->get_pos(), o->get_radius(), Line(frustum.pos(), frustum.dir().to_pt()));
	case F_Frustum:
		return intersect(o->get_pos(), o->get_radius(), frustum);
	case F_Rect:
		return intersect(rect.ll, rect.ur, Point2D(o->get_pos().x, o->get_pos().z), o->get_radius());
	default:
		break;
	}
	return false;
}

template<class C> void search_iterator<C>::erase()
{
	cont->erase(cur);
}

template class search_iterator<SearchCont<object::ShotPtr> >;

}
}


