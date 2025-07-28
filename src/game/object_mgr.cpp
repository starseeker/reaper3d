/*
 * $Author: fizzgig $
 * $Date: 2002/09/23 12:08:35 $
 * $Log: object_mgr.cpp,v $
 * Revision 1.50  2002/09/23 12:08:35  fizzgig
 * Moved all anonymous namespaces into the reaper-namespace
 *
 * Revision 1.49  2002/04/18 01:18:28  pstrand
 * scenario in data files now
 *
 * Revision 1.48  2002/04/16 19:58:41  pstrand
 * resource.h change
 *
 * Revision 1.47  2002/04/11 02:37:15  pstrand
 * remove objs from phys::engine
 *
 * Revision 1.46  2002/04/11 01:19:08  pstrand
 * res_stream exceptions
 *
 * Revision 1.45  2002/04/07 00:10:14  pstrand
 * *** empty log message ***
 *
 * Revision 1.44  2002/03/26 02:00:16  pstrand
 * preload
 *
 * Revision 1.43  2002/03/19 20:41:45  pstrand
 * reorg..
 *
 * Revision 1.42  2002/02/27 18:08:40  pstrand
 * release-0.96
 *
 * Revision 1.41  2002/02/18 11:49:21  peter
 * objektladdning
 *
 * Revision 1.40  2002/01/20 16:37:10  peter
 * hmm
 *
 * Revision 1.39  2001/12/11 17:46:53  peter
 * *** empty log message ***
 *
 * Revision 1.38  2001/12/03 00:38:32  peter
 * mer eventkonfig
 *
 * Revision 1.37  2001/12/02 21:46:44  peter
 * ''
 *
 * Revision 1.36  2001/12/02 21:34:01  peter
 * *** empty log message ***
 *
 * Revision 1.35  2001/12/02 17:40:46  peter
 * objmgrmeck
 *
 * Revision 1.34  2001/11/29 00:54:13  peter
 * 'erase'
 *
 * Revision 1.33  2001/11/27 13:10:59  peter
 * hata visual c+
 *
 * Revision 1.32  2001/11/27 04:29:39  peter
 * intelfixar
 *
 * Revision 1.31  2001/11/27 03:37:06  peter
 * object-death-callback (object manager..)
 *
 * Revision 1.30  2001/11/26 10:46:02  peter
 * kompilatorgnäll... ;)
 *
 */

#include "hw/compat.h"
#include "game/object_mgr.h"
#include "object/factory.h"
#include "object/object.h"
#include "object/phys.h"
#include "object/event.h"
#include "misc/parse.h"
#include "res/res.h"
#include <iostream>
#include "main/types_io.h"
#include "object/objectgroup.h"
#include "world/level.h"
#include "res/resource.h"
#include "phys/engine.h"



using namespace std;
using namespace reaper::misc;

namespace reaper {
namespace {
	reaper::debug::DebugOutput dout("scenario", 5);
}
namespace game {
namespace scenario {

using namespace object;

using res::resource;

ObjectMgr::ObjectMgr()
 : factory(factory::inst()),
   wr(world::World::get_ref()), 
   pr(reaper::phys::Engine::get_ref()),
   sh("objectmgr", this)
{
	init_loader("scenario");

}


/*
template<class Ptr, class Base, class F, class C>
void load_objs(F& f, C& c, const string& fn)
{
	try {
		res::res_stream g_is(res::Scenario, fn);

		do {
			Ptr g = f.load(g_is, TMap<Base>());
			if (g == 0)
				break;
			c.push_back(g);
		} while (true);
	}
	catch (res::resource_not_found) {
//		dout << "not found: " << r.what() << '\n';
	}
}
*/

void ObjectMgr::load_group(const std::string group)
{
	dout << "loading group " << group << '\n';
	res::resource<object::ObjectGroup>(group);
	for(int j = 0; j < Number_Companies; j++) {
		objs[std::make_pair(group, CompanyID(j))] = 0;
	}

/*
	string filename;

	load_objs<StaticPtr, StaticBase>(factory, o.turrets, scen + "_turrets");
	load_objs<PlayerPtr, PlayerBase>(factory, o.ships, scen + "_ships");
	load_objs<DynamicPtr, DynamicBase>(factory, o.gvs, scen + "_gvs");
	load_objs<SillyPtr, SillyBase>(factory, o.buildings, scen + "_buildings");
	load_objs<SillyPtr, SillyBase>(factory, o.nature, scen + "_nature");
*/
}

class OnDead
{
	ObjectCounter& count;
	std::string group;
	object::ID id;
	CompanyID cid;
public:
	OnDead(ObjectCounter& c, std::string g, object::ID i, CompanyID ci)
	 : count(c), group(g), id(i), cid(ci)
	{ }
	void operator()()
	{
		--count[std::make_pair(group, cid)];
		world::World::get_ref()->erase(id);
		reaper::phys::Engine::get_ref()->remove(id);
	}
};


class ObjAdder
{
	ObjectCounter& count;
	std::string group;
	reaper::phys::PhysRef pr;
public:
	ObjAdder(ObjectCounter& oc, std::string g, reaper::phys::PhysRef p)
	 : count(oc), group(g), pr(p)
	{ }
	void operator()(MkInfo mk)
	{
		// FIXME, well duh...

		SillyBase* ptr = object::factory::inst().gen_make(mk);
		if (PlayerBase* p = dynamic_cast<PlayerBase*>(ptr))
			pr->insert(PlayerPtr(p));
		else if (ShotBase* p = dynamic_cast<ShotBase*>(ptr))
			pr->insert(ShotPtr(p));
		else if (DynamicBase* p = dynamic_cast<DynamicBase*>(ptr))
			pr->insert(DynamicPtr(p));
		else if (StaticBase* p = dynamic_cast<StaticBase*>(ptr))
			pr->insert(StaticPtr(p));
		else if (SillyBase* p = dynamic_cast<SillyBase*>(ptr))
			pr->insert(SillyPtr(p));


		misc::Command<void> oc(misc::mk_cmd<void>(OnDead(count, group, ptr->get_id(), ptr->get_company())));
		ptr->events().death.add_listener(oc);
		++count[std::make_pair(group, ptr->get_company())];
	}
};


void ObjectMgr::add_group(const std::string g)
{
	dout << "adding group " << g << '\n';
	object::ObjectGroup& og = res::resource<object::ObjectGroup>(g);
	for_each(og.objs.begin(), og.objs.end(), ObjAdder(objs, g, pr));
}

struct {
	template<class T>
	void operator()(T t) { t->think(); }
} think_t;

void ObjectMgr::think(void)
{
	for_each(wr->begin_st(), wr->end_st(), think_t);
	for_each(wr->begin_dyn(), wr->end_dyn(), think_t);
}


void ObjectMgr::shutdown()
{
}

int ObjectMgr::nr_in_group(const std::string& group, CompanyID cid)
{
	return objs[std::make_pair(group, cid)];
}

void ObjectMgr::dump(state::Env& env) const
{
}

void ObjectMgr::get_objectnames(std::set<std::string>& s)
{
	ObjectCounter::iterator c, e = objs.end();
	for (c = objs.begin(); c != e; ++c) {
		ObjectGroup& og
			= res::resource<ObjectGroup>(c->first.first);
		MkInfoVec::iterator m, me = og.objs.end();
		for (m = og.objs.begin(); m != me; ++m) {
			s.insert(m->name);
		}
	}
}



}
}
}



