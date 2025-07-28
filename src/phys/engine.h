#ifndef PHYS_PHYSENGINE2_H
#define PHYS_PHYSENGINE2_H

#include "hw/compat.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "misc/hash.h"

#include "misc/uniqueptr.h"
#include "object/baseptr.h"
#include "world/worldref.h"
#include "game/state.h"
#include "main/types.h"
#include "phys/collisionQueue.h"

#include <vector>
using std::vector;

#include "misc/uniqueptr.h"

using namespace reaper;
using namespace reaper::object;
using reaper::misc::hash_map;

namespace reaper{
typedef int objId;
   
namespace phys{

class Engine;
typedef reaper::misc::UniquePtr<reaper::phys::Engine> PhysRef;

class Engine  
{
public:
	~Engine();

	void update_world(double start_time, double delta_time);

	void insert(ShotPtr shot);
	void insert(ShotPtr shot,double sim_time);
	void insert(PlayerPtr ply);
	void insert(DynamicPtr dyn);
	void insert(SillyPtr sil);
	void insert(StaticPtr stat);

	void remove(objId id);

	static PhysRef create();
	static PhysRef get_ref();

	int size();
	void shutdown();
	void reinit();

private:
	class StateRestore : public game::state::Dump
	{
	public:
		void dump(game::state::Env&) const;
	} state_restore;

	Engine();

	bool to_insert(double framestop,const Pair& p);
	void startup();
	void clean_dead_objects();

	reaper::world::WorldRef wr;
	
	double frame_stop;
	bool simulating;
	float tri_interval;
	float since_last_tri;

	float last_garbage;

	hash_map< objId, double >  dead_objects;
	PriorityQueue prio_queue;
	
	std::deque<SillyPtr>   sillys;
	std::deque<StaticPtr>  statics;
	std::deque<DynamicPtr> dynamics;
	std::deque<ShotPtr>    shots;
	
	friend class reaper::misc::UniquePtr<reaper::phys::Engine>;
};

}
}

#endif
