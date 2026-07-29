#include "phys/engine.h"
#include "world/world.h"
#include "phys/exceptions.h"

using reaper::phys::phys_error;
using reaper::phys::phys_fatal_error;
#include "main/types_ops.h"
#include "world/world.h"
#include "world/level.h"
#include "object/phys.h"
#include "misc/smartptr.h"

#include <iostream>
using std::cout;
using std::endl;
#include <algorithm>
#include <set>
using std::find;
using std::set;

using namespace reaper::world;

#include "main/types_io.h"

namespace reaper {
namespace {
	debug::DebugOutput dout("phys::Engine", 5);
}
namespace phys {

#include <time.h>

inline double stopwatch() {return (double)clock()/(double)CLOCKS_PER_SEC;}

Engine::Engine():
wr(reaper::world::World::get_ref()),
frame_stop(0),
simulating(false),
tri_interval(1.0),
since_last_tri(tri_interval+0.1),
last_garbage(0.0)
{
	if(!wr.valid()) {
		throw phys_fatal_error("phys::Engine::Engine() World not created!");
	}
//	dout << "created\n";
	
	startup();
}

PhysRef Engine::create()
{
	return PhysRef::create();
}

PhysRef Engine::get_ref()
{
	return PhysRef();
}


void Engine::update_world(double start_time, double delta_time)
{

	frame_stop = start_time + delta_time;
	simulating = true;

	since_last_tri += delta_time;
	
	for(dyn_iterator i = wr->begin_dyn();i != wr->end_dyn(); ++i) {
		DynamicPtr d = *i;
		object::phys::ObjectAccessor &phys_acc = d->get_physics();
		
		float radius = phys_acc.radius;
		//dout << "Radius: " << radius << std::endl;
		radius 
			+= length(d->get_velocity())*delta_time
			+ phys_acc.max_acc*pow(delta_time,2);
		
		tri_iterator tr(wr->find_tri(Sphere(d->get_pos(),radius)));
		
		for( ; tr != wr->end_tri(); ++tr){
			auto objtri = std::make_shared<DynTriPair>(d, *tr);
			objtri->calc_lower_bound();
			if(objtri->get_lower_bound() < frame_stop)
				prio_queue.push(std::move(objtri));
			
		}				
	}
	

	if(!prio_queue.empty()) {
		while( !prio_queue.empty() && prio_queue.top()->get_lower_bound() < frame_stop ) {
			
			//Now check if we want to simulate a collision between objects                                 
			PairPtr pair = prio_queue.top();

			//the object will be simulated this far when the loop has ended

			prio_queue.pop();
			//Pair simulate simulates until lower_bound 
			if(dead_objects.count(pair->get_id1()) != 0 ||
			   dead_objects.count(pair->get_id2()) != 0) {
			}
			else{
				pair->simulate( pair->get_lower_bound() );                                        
				
				if(pair->check_distance() < Collision_Distance ) {
					const CollisionData collision =
						pair->get_collision();
					pair->collide(collision);
				}				
				
				pair->calc_lower_bound();
				if(pair->to_insert(frame_stop)  )
					prio_queue.push(std::move(pair));
				
			}
		}

	}
		
	//Now simulate all objects until framestop
	//Simulate all aobjekt so that they have a simtime of end of fram		
	for(dyn_iterator i(wr->begin_dyn()); i != wr->end_dyn(); ++i){
		double dt = frame_stop - (*i)->get_sim_time();
		if(dt > 0)
			(*i)->simulate(dt);
	}
	
	for(st_iterator i(wr->begin_st()); i != wr->end_st(); ++i){
		double dt = frame_stop - (*i)->get_sim_time();
		if(dt > 0)
			(*i)->simulate(dt);
	}
	
	for(shot_iterator i(wr->begin_shot()); i != wr->end_shot(); ++i){
		double dt = frame_stop - (*i)->get_sim_time();
		if(dt > 0) {
//			dout << "simul: " << (*i).get() << '\n';
			(*i)->simulate(dt);
		}
	}

	simulating = false;
	
	//Inserting all objects
	for(std::deque<ShotPtr>::iterator i    = shots.begin();    i != shots.end(); ++i)    { insert(*i); }
	for(std::deque<SillyPtr>::iterator i   = sillys.begin();   i != sillys.end(); ++i)   { insert(*i); }
	for(std::deque<StaticPtr>::iterator i  = statics.begin();  i != statics.end(); ++i)  { insert(*i); }
	for(std::deque<DynamicPtr>::iterator i = dynamics.begin(); i != dynamics.end(); ++i) { insert(*i); }
	

	if(last_garbage + Maximum_Lower_bound < frame_stop){
		clean_dead_objects();
		last_garbage = frame_stop;
	}

	sillys.clear();
	statics.clear();
	dynamics.clear();
	shots.clear();

}

void Engine::remove(objId id)
{
	dead_objects[id] = frame_stop + Maximum_Lower_bound;
}

void Engine::clean_dead_objects()
{
	for (auto it = dead_objects.begin(); it != dead_objects.end();) {
		if(it->second < frame_stop)
			it = dead_objects.erase(it);
		else
			++it;
	}
}

void Engine::startup()
{
	
	for(dyn_iterator dyn(wr->begin_dyn()); dyn != wr->end_dyn(); ++dyn) {
		dyn_iterator i(dyn);
		++i;
		for(; i != wr->end_dyn();++i) {
			auto pair = std::make_shared<DynDynPair>(*dyn, *i);
			pair->calc_lower_bound();
			prio_queue.push(std::move(pair));
		}
	}

	
	for(dyn_iterator dyn(wr->begin_dyn()); dyn != wr->end_dyn(); ++dyn) {
		for(st_iterator i(wr->begin_st()); i != wr->end_st(); ++i) {
			auto pair = std::make_shared<StaticDynPair>(*i, *dyn);
			pair->calc_lower_bound();
			prio_queue.push(std::move(pair));
		}
	}

	
	for(dyn_iterator dyn(wr->begin_dyn()); dyn != wr->end_dyn(); ++dyn) {
		for(si_iterator i(wr->begin_si()); i != wr->end_si(); ++i) {
			auto pair = std::make_shared<SillyDynPair>(*i, *dyn);
			pair->calc_lower_bound();
			prio_queue.push(std::move(pair));
		}
	}
}

void Engine::insert(ShotPtr shot)
{
	if(simulating)
		shots.push_back(shot);
	else
		insert(shot,frame_stop);
	
}


void Engine::insert(ShotPtr shot,double sim_time)
{
//	const double& simt = shot->get_sim_time();
	
	//shot->simulate(frame_stop - simt);
	//shot->set_sim_time(frame_stop);
	
	//TEST to simulate shots here
	
	float dies_at = shot->get_sim_time();
	
	dies_at += shot->get_lifespan(); 
	
	
	//Get all objects inside a frustum

	using reaper::world::Line;
	using reaper::world::Frustum;
	
	const Point p1 = shot->get_pos();
	const Point p2(shot->get_pos() + shot->get_velocity()*shot->get_lifespan());
	
	//Some geomterics for the object
	Line line(p1,p2);
	Frustum dyn_frust(shot->get_pos(),
			  shot->get_velocity()*(shot->get_lifespan() + 1),
			  shot->get_mtx().col(1),
			  45.0, 45.0);

	object::ID parent = shot->get_parent();

	world::dyn_iterator di(wr->find_dyn(dyn_frust));

	for( ; di != wr->end_dyn(); ++di) {
		if((*di)->get_id() != parent) {
			auto pair = std::make_shared<ShotDynPair>(shot, *di);
			pair->calc_lower_bound();
			
			if(pair->get_lower_bound() < dies_at )
				prio_queue.push(std::move(pair));
		}
	}
	
	world::st_iterator st(wr->find_st(line));

	for(; st != wr->end_st(); ++st) {
		if((*st)->get_id() != parent) {
			auto pair = std::make_shared<ShotSillyPair>(shot, *st);
			pair->calc_lower_bound();
			
			if(pair->get_lower_bound() < dies_at)
				prio_queue.push(std::move(pair));
		}
	}
	
	
	world::si_iterator si(wr->find_si(line));


	for (; si != wr->end_si(); ++si) {
		if ((*si)->get_id() != parent){
			auto pair = std::make_shared<ShotSillyPair>(shot, *si);
			pair->calc_lower_bound();
			
			if(pair->get_lower_bound() < dies_at)
				prio_queue.push(std::move(pair));
		}
	}
	
	world::tri_iterator tri(wr->find_tri(line));

	for( ; tri != wr->end_tri(); ++tri){
		auto pair = std::make_shared<ShotTriPair>(shot, *tri);
		pair->calc_lower_bound();
		float lb = pair->get_lower_bound();
		if (lb < dies_at) {
			prio_queue.push(std::move(pair));
		}
	}
	
	
	wr->add_object(shot);

}

void Engine::insert(PlayerPtr p)
{
	insert(DynamicPtr(p));
}

template<class PairType, class Prio, class Iter>
void create_pairs(Prio& prio_queue, DynamicPtr dyn, Iter begin, Iter end)
{
	for (Iter i = begin; i != end; ++i) {
		if (length(dyn->get_pos() - (*i)->get_pos())
		      - (dyn->get_radius() + (*i)->get_radius())
		     < Collision_Distance) {
			auto pair = std::make_shared<PairType>(*i, dyn);
			pair->calc_lower_bound();
			prio_queue.push(std::move(pair));
		}
	}
}

void Engine::insert(DynamicPtr dyn)
{
	if(simulating){
		dynamics.push_back(dyn);
		return;
	}
	
	dyn->set_sim_time(frame_stop);

	create_pairs<DynDynPair>(prio_queue, dyn, wr->begin_dyn(), wr->end_dyn());
	create_pairs<StaticDynPair>(prio_queue, dyn, wr->begin_st(), wr->end_st());
	create_pairs<SillyDynPair>(prio_queue, dyn, wr->begin_si(), wr->end_si());

	wr->add_object(dyn);
}

void Engine::insert(SillyPtr sil)
{
	if(simulating){
		sillys.push_back(sil);
		return;
	}
		
	
	for(dyn_iterator i(wr->begin_dyn()); i != wr->end_dyn(); ++i) {
		if(length(sil->get_pos() - (*i)->get_pos()) -
			(sil->get_radius() + (*i)->get_radius() )
			< Collision_Distance){
			auto pair = std::make_shared<SillyDynPair>(sil, *i);
			pair->calc_lower_bound();
			prio_queue.push(std::move(pair));
		}
	}
	wr->add_object(sil);

}

void Engine::insert(StaticPtr stat)
{
	if(simulating){
		statics.push_back(stat);
		return;
	}
	
	stat->set_sim_time(frame_stop);
	
	for(dyn_iterator i(wr->begin_dyn()); i!= wr->end_dyn(); ++i){
		if(length(stat->get_pos() - (*i)->get_pos()) -
			(stat->get_radius() + (*i)-> get_radius() )
			< Collision_Distance){
			auto pair = std::make_shared<StaticDynPair>(stat, *i);
			pair->calc_lower_bound();
			prio_queue.push(std::move(pair));
		}
	}
	wr->add_object(stat);

}

int Engine::size()
{
	return prio_queue.size();
}

void clear(PriorityQueue& pairs)
{
	while(!pairs.empty())
		pairs.pop();
}

void Engine::shutdown()
{
	clear(prio_queue);
	sillys.clear();
	statics.clear();
	dynamics.clear();
	shots.clear();
}

void Engine::reinit()
{
	dout << "Rebuilding pairs\n";
	clear(prio_queue);
	startup();
}

Engine::~Engine()
{
	dout << "destroyed\n";
	clear(prio_queue);
//	PhysRef::destroy();
}

void Engine::StateRestore::dump(game::state::Env&) const
{
}

}//phys
}//reaper
