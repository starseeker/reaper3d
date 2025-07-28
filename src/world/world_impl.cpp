/* $Id: world_impl.cpp,v 1.11 2002/09/23 12:08:44 fizzgig Exp $ */

#include "hw/compat.h"

#include "world/world.h"

#include "game/state.h"
#include "hw/debug.h"
#include "misc/free.h"
#include "misc/iostream_helper.h"
#include "misc/sequence.h"
#include "misc/stlhelper.h"
#include "object/base.h"
#include "object/factory.h"
#include "world/exceptions.h"
#include "world/level.h"
#include "world/world_impl.h"
#include "res/res.h"

#include <queue>


namespace reaper {
namespace { 
    reaper::debug::DebugOutput dlog("world", 5); 
}
namespace object { 
    int get_obj_count(); 
}
namespace world {       


using misc::lower_bound;
using misc::upper_bound;
using misc::seq;
using misc::for_each;
using misc::copy;
using misc::sort;
using std::mem_fun_ref;
using misc::Seq;

using object::SillyPtr;
using object::StaticPtr;
using object::DynamicPtr;
using object::ShotPtr;
using object::PlayerPtr;


namespace {
	object::ID next_id = 10000;
}

void reset_id()
{
	next_id = 10000;
}

object::ID alloc_id()
{
	return next_id++;
}




World_impl::World_impl(const std::string& level)
 : tri_tree(0), si_tree(0), st_tree(0), dyn_tree(0), 
   // sthold("world", this),
   level_name(level), in_shutdown(false)
{
	reset_id();
	init();
}


World_impl::~World_impl() {
//		dlog << "dyn world size: " << *dyn_objs.front().get().getcount().count
//		     << ' ' << dyn_objs.size() << '\n';
//		dlog << "shot world size: " << *dyn_objs.front().get().getcount().count
//		     << ' ' << shot_objs.size() << '\n';
//		for_each(seq(dyn_objs), mem_fun_ref(&Box<DynamicPtr>::dealloc));
//		for_each(seq(shot_objs), mem_fun_ref(&Box<ShotPtr>::dealloc));
	dlog << "~World_impl objects: " << object::get_obj_count() << '\n';
}

void World_impl::init()
{
	using std::max;
	level_info = new LevelInfo(level_name);
	float min_x = level_info->terrain_min_x;
	float min_z = level_info->terrain_min_z;
	float dim = max(level_info->terrain_max_x - level_info->terrain_min_x,
			level_info->terrain_max_x - level_info->terrain_min_x);

	tri_tree = new tri_container(min_x, min_z, dim);
	si_tree  = new si_container(min_x, min_z, dim);
	st_tree  = new st_container(min_x, min_z, dim);
	dyn_tree = new dyn_container(min_x, min_z, dim);
	shot_tree = new shot_container(min_x, min_z, dim);

}

class ostream_out
{
	std::ostream& os;
public:
	ostream_out(std::ostream& o) : os(o) { }
	template<class T>
	void operator()(const T& t)
	{
		t->dump(os);
		os << '\n';
	}

};

void World_impl::dump(game::state::Env& env) const
{
	env["level"] = level_name;
	env["local_player"] = local_player->get_id();


	std::auto_ptr<res::res_out_stream> os = game::state::statemgr().obj_store_stream();
	
	*os << "[silly]\n";
	misc::for_each(misc::seq(*si_tree), ostream_out(*os));
	*os << "[static]\n";
	misc::for_each(misc::seq(*st_tree), ostream_out(*os));
	*os << "[dyn]\n";
	misc::for_each(misc::seq(*dyn_tree), ostream_out(*os));
	*os << "[shot]\n";
	misc::for_each(misc::seq(*shot_tree), ostream_out(*os));
}

template<class T>
void chk_ld(std::istream& is, const std::string& id, T& t)
{
	misc::ConfigLine<T> cl;
	is >> cl;
	if (cl.var != id)
		throw world_format_error(cl.var + " - " + id);
	t = cl.val;
}

template<class Base, class F>
void load_objs(std::istream& is, F& f, TMap<Base>)
{
	do {
		misc::SmartPtr<Base> g = f.load(is, TMap<Base>());
		World::get_ref()->add_object(g);
	} while (is.peek() != '[');
}


void World_impl::restore(std::istream& is)
{
	object::factory::Factory& f = object::factory::inst();
	chk_ld(is, "level", level_name);
	init();
	object::ID id;
	chk_ld(is, "local_player", id);
	is >> misc::crlf;
	while (!isspace(is.peek())) {
		char junk;
		std::string type;
		is >> junk >> type >> junk >> misc::crlf;
		if (type == "silly")
			load_objs(is, f, TMap<object::SillyBase>());
		if (type == "static")
			load_objs(is, f, TMap<object::StaticBase>());
		if (type == "dyn")
			load_objs(is, f, TMap<object::DynamicBase>());
		if (type == "shot")
			load_objs(is, f, TMap<object::ShotBase>());
	}
	set_local_player(id);
}


void World_impl::set_local_player(object::ID id)
{
	DynamicPtr d = dynamics[id];
	if (!d.valid())
		throw world_error("get_local_player: not found");
	local_player.dynamic_assign(d);
	if (!local_player.valid())
		throw world_error("get_local_player: not a player object!");
}

void World_impl::shutdown()
{
	in_shutdown = true;
	for_each(seq(*tri_tree), misc::delete_it);

	misc::zero_delete(tri_tree);
	misc::zero_delete(st_tree);
	misc::zero_delete(si_tree);
	misc::zero_delete(dyn_tree);
	misc::zero_delete(shot_tree);
	misc::zero_delete(level_info);
	local_player.invalidate();
	sillys.clear();
	statics.clear();
	dynamics.clear();
	shots.clear();
	dlog << "objects after shutdown: " << object::get_obj_count() << '\n';
}





}

}
