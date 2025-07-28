
#ifndef REAPER_WORLD_WORLD_IMPL_H
#define REAPER_WORLD_WORLD_IMPL_H

#include "hw/compat.h"

#include "world/world.h"

#include "game/state.h"
#include "world/exceptions.h"
#include "world/level.h"
#include "object/base.h"
#include "misc/hash.h"

namespace reaper {
namespace world {

using object::SillyPtr;
using object::StaticPtr;
using object::DynamicPtr;
using object::ShotPtr;
using object::PlayerPtr;


typedef quadtree::QuadTree<Triangle*> tri_container;
typedef quadtree::QuadTree<object::SillyPtr> si_container;
typedef quadtree::QuadTree<object::StaticPtr> st_container;
typedef quadtree::QuadTree<object::DynamicPtr> dyn_container;
typedef quadtree::QuadTree<object::ShotPtr> shot_container;

struct World_impl  : public game::state::Persistent
{
	tri_container* tri_tree;
	si_container* si_tree;
	st_container* st_tree;
	dyn_container* dyn_tree;
	shot_container* shot_tree;

	misc::hash_map<object::ID, SillyPtr> sillys;
	misc::hash_map<object::ID, StaticPtr> statics;
	misc::hash_map<object::ID, DynamicPtr> dynamics;
	misc::hash_map<object::ID, ShotPtr> shots;

	PlayerPtr local_player;

	LevelInfo* level_info;

//	game::StateHolder sthold;
	std::string level_name;

	bool in_shutdown;

	World_impl(const std::string& level);
	World_impl(const LevelInfo& level);
	~World_impl();

	void init();

	void dump(game::state::Env& env) const;

	void restore(std::istream& is);

	void shutdown();

	void set_local_player(object::ID);
};

}
}


#endif


