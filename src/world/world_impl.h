
#ifndef REAPER_WORLD_WORLD_IMPL_H
#define REAPER_WORLD_WORLD_IMPL_H


#include "world/world.h"

#include "game/state.h"
#include "world/exceptions.h"
#include "world/level.h"
#include "object/base.h"
#include <memory>
#include <unordered_map>

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
	std::unique_ptr<tri_container> tri_tree;
	std::unique_ptr<si_container> si_tree;
	std::unique_ptr<st_container> st_tree;
	std::unique_ptr<dyn_container> dyn_tree;
	std::unique_ptr<shot_container> shot_tree;

	std::unordered_map<object::ID, SillyPtr> sillys;
	std::unordered_map<object::ID, StaticPtr> statics;
	std::unordered_map<object::ID, DynamicPtr> dynamics;
	std::unordered_map<object::ID, ShotPtr> shots;

	PlayerPtr local_player;

	std::unique_ptr<LevelInfo> level_info;

//	game::StateHolder sthold;
	std::string level_name;

	bool in_shutdown = false;

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
