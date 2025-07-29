/*
 * Author: Peter Strand <d98peter@dtek.chalmers.se>
 *
 * Highlevel network management,
 * and object synchonization.
 *
 */

#include "hw/compat.h"

#include <vector>
#include <string>
#include <iostream>
#include <queue>

#include "hw/debug.h"
#include "hw/event.h"
#include "hw/reltime.h"
#include "hw/worker.h"
#include "hw/concurrent.h"

#include "net/net.h"
#include "misc/parse.h"
#include "misc/free.h"
#include "misc/sequence.h"
#include "res/config.h"

#include "net/game_mgr.h"
#include "net/netgame.h"
#include "net/net.h"

#include "net/server.h"

#include "object/phys.h"
#include "object/base_data.h"

#include "world/world.h"
#include "main/types.h"
#include "main/types_io.h"

namespace reaper {
namespace net {

namespace { debug::DebugOutput derr("game_mgr", 5); }

using namespace misc;
using std::map;
using std::string;
using std::deque;

typedef map<object::ID, deque<ObjState> > SaveObjStates;
typedef SaveObjStates::iterator save_state_iter;

struct GameMgr_impl
{
	NetGame net;
	hw::event::EventSystem es;

	world::WorldRef wr;

	deque<object::ID> local_ids;

	IntObjStates obj_states;
	SaveObjStates saved_obj_states;

	hw::time::RelTime last_sync_s, last_sync_r, last_save;
	hw::time::RelTime sync_interval;
	hw::event::EventFilter* ef;
	string map;

	GameMgr_impl(hw::gfx::Gfx& gx)
	 : es(gx), 
	   last_sync_s(0), last_sync_r(0), last_save(0),
	   sync_interval(100), ef(0)
	{
		es.disable();
	}

	~GameMgr_impl()
	{
	}

	void diffify(const ObjState&);

	void save_state();
	void send_sync();
	void recv_sync();


	void init_game(bool netw, std::string srv, bool split)
	{
		if (netw)
			ef = net.connect(srv);
		map = (split) ? "hw_event_split_map" : "hw_event_game_map";
	}
};

void diff(Matrix& d, const Matrix& m1, const Matrix& m2)
{	
	d.col(0, m1.col(0) - m2.col(0));
	d.col(1, m1.col(1) - m2.col(1));
	d.col(2, m1.col(2) - m2.col(2));
	d.pos() = m1.pos() - m2.pos();
}

bool diff_to_big(const Matrix& m)
{
	Vector c_x = m.col(0);
	Vector c_y = m.col(1);
	Vector c_z = m.col(2);
	Point p = m.pos();
	return dot(c_x, c_x) > 0.5
	    || dot(c_y, c_y) > 0.5
	    || dot(c_z, c_z) > 0.5
	    || dot(p, p) > 100;
}

bool diff_to_big(const Vector& v)
{
	return dot(v, v) > 10; 
}

void GameMgr_impl::diffify(const ObjState& st)
{
	deque<ObjState>& sv = saved_obj_states[st.id];

	if (sv.empty()) {
		derr << "Warning, no object state saved for " << st.id << '\n';
		return;
	}
	while (st.sync > sv.front().sync) {
		derr << "throwing away: " << sv.front().sync << " (< " << st.sync << ")\n";
		sv.pop_front();
		if (sv.empty())
			return;
	}
	const ObjState& loc_obj = sv.front();
//	derr << "Diffing " << st.id << ' ' << st.sync << " against " << loc_obj.sync << '\n';

	NetObjPtr p = obj_states[st.id];
	object::phys::ObjectAccessor& oa = p->get_physics();

	if (st.dead) {
		if (!loc_obj.dead) {
			derr << "killing obj: " << p->get_id() << '\n';
			p->kill();
		}
		int_state_iter i = obj_states.find(p->get_id());
		if (i != obj_states.end())
			obj_states.erase(i);
		else
			derr << "doomed obj " << p->get_id() << " already killed?\n";
	} else {
		if (loc_obj.dead) {
			derr << "obj " << p->get_id() << " locally dead but not on server! (sending death order)\n";
			net.send_objinfo(loc_obj);
		}
	}


	if (st.sync < sv.front().sync)
		return;


	Matrix dm;
	diff(dm, st.mat, loc_obj.mat);
	Vector dvv = st.vel - loc_obj.vel;
	Vector dvr = st.rot_vel - loc_obj.rot_vel;

//	derr << "diff: " << dm << ' ' << dvv << ' ' << dvr << '\n';

	Matrix om;
	if (diff_to_big(dm)) {
		om = st.mat;
	} else {
		om = oa.get_mtx() + dm;
		orthonormalize3(om);
	}
	oa.set_mtx(om);

	oa.vel     = diff_to_big(dvv) ? static_cast<Vector>(st.vel)     : oa.vel + dvv;
	oa.rot_vel = diff_to_big(dvr) ? static_cast<Vector>(st.rot_vel) : oa.rot_vel + dvr;

	sv.pop_front();
}

void GameMgr_impl::recv_sync()
{
	hw::time::RelTime now = hw::time::get_rel_time();
	if (now < (last_sync_r + (sync_interval)))
		return;
	last_sync_r = now;
//	derr << "Running recv: " << now << '\n';

	ObjState st;
	while (net.get_objinfo(st))
		diffify(st);
}

void GameMgr_impl::save_state()
{
	hw::time::RelTime now = hw::time::get_rel_time();
	if (now < (last_save + sync_interval))
		return;
	last_save = now;

	for (int_state_iter c = obj_states.begin(); c != obj_states.end(); ++c) {
///		if (find(seq(local_ids), c->first) != local_ids.end())
//			continue;

		object::phys::ObjectAccessor& oa = c->second->get_physics();
		
		saved_obj_states[c->first].push_back(
			ObjState(c->first, now, oa.get_mtx(), oa.vel, oa.rot_vel, c->second->is_dead()));
//		derr << "saved states: " << saved_obj_states.size()
//		     << " x " << saved_obj_states[c->first].size()
//		     << "  time: " << now << '\n';

	}

}

void GameMgr_impl::send_sync()
{
	hw::time::RelTime now = hw::time::get_rel_time();
	if (now < (last_sync_s + sync_interval))
		return;
       	last_sync_s = now;

	std::deque<object::ID>::iterator c, e = local_ids.end();
	for (c = local_ids.begin(); c != e; ++c) {
		if (saved_obj_states[*c].empty())
			continue;
		net.send_objinfo(saved_obj_states[*c].back());
	}
}

GameMgr::GameMgr(hw::gfx::Gfx& gx)
{
	impl = new GameMgr_impl(gx);
}

GameMgr::~GameMgr()
{
	if (impl)
		delete impl;
}


class ServerJob : public hw::worker::Job {
	net::GameServer gs;
public:
	bool operator()() {
		gs.run();
		return false;
	}
};

void GameMgr::start_server()
{
	hw::worker::worker()->spawn_job(new ServerJob());
}

void GameMgr::add_sync(NetObjPtr o)
{
//	derr << "new obj: " << o.get() << '\n';
	impl->obj_states[o->get_id()] = o;
}

void GameMgr::update()
{
	if (!impl->net.is_connected() || impl->obj_states.empty())
		return;

	impl->save_state();

	impl->send_sync();
	impl->recv_sync();
}


void GameMgr::init(bool netw, std::string srv, bool split)
{
	impl->init_game(netw, srv, split);
}


PlayerID GameMgr::alloc_id(bool observer)
{
	PlayerID id = impl->net.join(observer);
	if (!observer)
		impl->local_ids.push_back(id);
	return id;
}


const Players& GameMgr::players()
{
	return impl->net.players();
}

bool GameMgr::get_multistatus()
{
	return impl->net.get_multistatus();
}


bool GameMgr::start_req()
{
	if (impl->net.is_connected())
		return impl->net.start_net_game();
	else
		return true;
}

void GameMgr::end()
{
	impl->net.shutdown();
}


void GameMgr::go_go_go()
{
	impl->es.set_mapping(impl->map);
	if (impl->net.is_connected()) {
		impl->net.sync_start();
		impl->es.add_filter(impl->ef);
	}
	hw::time::reset_rel_time();
	impl->es.enable();
}

void GameMgr::record_game(bool b)
{
	if (b)
		impl->es.add_filter(hw::event::make_savedevice());
}

void GameMgr::playback_game(const string& id)
{
	if (!id.empty())
		impl->es.add_filter(hw::event::make_playbackdevice(id));
}

void GameMgr::shutdown()
{
	misc::zero_delete(impl);
}


}
}

