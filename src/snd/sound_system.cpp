
/* $Id: sound_system.cpp,v 1.57 2002/05/21 10:09:47 pstrand Exp $ */

#include "hw/compat.h"

#include <vector>
#include <map>
#include <string>
#include <queue>

#include "misc/smartptr.h"
#include "misc/free.h"
#include "hw/debug.h"
#include "hw/snd.h"
#include "snd/sound_system.h"
#include "main/types_io.h"
#include "misc/stlhelper.h"
#include "world/world.h"
#include "object/base.h"
#include "hw/reltime.h"

namespace reaper
{
namespace misc {
	template <>
	UniquePtr<sound::Manager>::I UniquePtr<sound::Manager>::inst;
}
namespace sound
{

const int wrap_snd_dist_max = 400;
const int wrap_snd_dist_min = 300;
const int max_sound_distance = 600;
const int max_pos_sound_distance = 800;

const float max_new_pos_snd_per_sec = 24;
const float max_new_move_snd_per_sec = 12;


class DummyEff
 : public hw::snd::Effect
{
public:
	DummyEff() { }
	~DummyEff() { }
	void play() { }
	void stop() { }
	void set_position(const Point& pos) { }
	void set_volume(float vol) { }
	void set_direction(const Vector& d) { }
	void set_velocity(const Vector& v) { }
	void set_pitch(float p) { }
	void set_loop() { }

	EffectPtr clone() const {
		return EffectPtr(new DummyEff());
	}
};

class Wrap
 : public hw::snd::Effect
{
protected:
	EffectSPtr eff;
public:
	Wrap(EffectSPtr e) : eff(e) { }
	~Wrap() { }
	void play() { eff->play(); }
	void stop() { eff->stop(); }
	void set_position(const Point& pos) { eff->set_position(pos); }
	void set_volume(float vol) { eff->set_volume(vol); }
	void set_direction(const Vector& d) { eff->set_direction(d); }
	void set_velocity(const Vector& v) { eff->set_velocity(v); }
	void set_pitch(float p) { eff->set_pitch(p); }
	void set_loop() { eff->set_loop(); }

	EffectPtr clone() const {
		return EffectPtr(new Wrap(eff));
	}
};

class DistWrap : public Wrap
{
	bool active, enable;
	Point* cam; // FIXME evil
public:
	DistWrap(EffectSPtr eff, Point* c)
	 : Wrap(eff), active(false), enable(false), cam(c)
	{ }
	void play() {
		active = true;
		if (enable) {
			eff->play();
		}
	}
	void stop() {
		active = false;
		eff->stop();
	}
	void set_position(const Point& pos)
	{
		float dist = length(pos - *cam);
		eff->set_position(pos);

		if (dist > wrap_snd_dist_max) {
			enable = false;
			eff->stop();
		} else if (dist < wrap_snd_dist_min) {
			if (active && !enable)
				eff->play();
			enable = true;
		}
	}
	EffectPtr clone() const {
		return EffectPtr(new DistWrap(eff, cam));
	}
};

using namespace std;

typedef map<string, EffectSPtr> Cache;

class Internal
{
	friend class Interface;
	friend class Manager;
	world::Cylinder cyl;
	hw::snd::SoundSystem ss;

	Cache cache;
	queue<pair<int, EffectSPtr> > death_list;

	bool sound_ok;
	Point cam_pos;

	int count_pos, count_move;
	hw::time::RelTime last;

	int tick;
public:
	Internal()
	 : cyl(Point2D(0, 0), 600), count_pos(0), count_move(0), last(0), tick(0)
	{
		sound_ok = ss.init();
		ss.set_volume(0);
	}
	~Internal()
	{
	}
	EffectSPtr load(const string& id, float vol, bool loop)
	{
		Cache::iterator i = cache.find(id);
		EffectSPtr eff;
		if (i != cache.end())
			eff = EffectSPtr(i->second->clone().release());
		else {
			eff = EffectSPtr(ss.prepare_effect(id).release());
			cache[id] = EffectSPtr(eff->clone().release());
		}
		eff->set_volume(vol);
		if (loop)
			eff->set_loop();
		return eff;
	}
	void shutdown()
	{
		while (!death_list.empty()) {
			death_list.pop();
		}
	}
};


EffectPtr Manager::load(const string& id, float vol, bool loop)
{
	if (snd_int->count_move++ > max_new_move_snd_per_sec) {
		return EffectPtr(new DummyEff());
	} else {
		EffectSPtr eff = snd_int->load(id, vol, loop);
		return EffectPtr(new DistWrap(eff, &snd_int->cam_pos));
	}
}

EffectPtr Manager::load(const string& id, const Point& pos, float vol)
{
	if ((snd_int->count_pos++ > max_new_pos_snd_per_sec) || 
	    (length(pos - snd_int->cam_pos) > max_pos_sound_distance))
		return EffectPtr(new DummyEff());

	EffectSPtr eff = snd_int->load(id, vol, false);
	eff->set_position(pos);
	return EffectPtr(new Wrap(eff));
}


void Manager::play(const string& id, const Point& pos, float v)
{
	EffectPtr eff = load(id, pos, v);
	eff->play();
	snd_int->death_list.push(make_pair(snd_int->tick, eff.release()));
}



Manager::Manager()
 : snd_int(new Internal())
{
	set_camera(Point(0,0,0), Vector(0,0,1), Vector(0,0,0));
}

Manager::~Manager()
{
	shutdown();
	delete snd_int;
//	SoundRef::force_death();
}

SoundRef Manager::create() { return SoundRef::create(); }
SoundRef Manager::get_ref()  { return SoundRef(); }



void Manager::set_camera(const Point& pos, const Vector& dir, const Vector& vel)
{
	snd_int->cam_pos = pos;
	snd_int->cyl.p = Point2D(pos.x, pos.z);
	snd_int->ss.set_listener(pos, dir, vel);
}


template<class Obj>
struct mk_snd {
	int operator()(Obj o) {
		o->gen_sound();
		return 0;
	}
};

void Manager::run()
{
	if (!snd_int->sound_ok)
		return;
	hw::time::RelTime now = hw::time::get_rel_time();
	if (now > (snd_int->last + 1e3)) {
		snd_int->count_move = snd_int->count_pos = 0;
		snd_int->last = now;
		snd_int->tick++;
	}
	while (! snd_int->death_list.empty() && snd_int->death_list.front().first < snd_int->tick - 2) {
		snd_int->death_list.pop();
	}
	world::WorldRef wr = world::World::get_ref();
	for_each(wr->find_st(snd_int->cyl), wr->end_st(), mk_snd<object::StaticPtr>());
	for_each(wr->find_dyn(snd_int->cyl), wr->end_dyn(), mk_snd<object::DynamicPtr>());
	for_each(wr->find_shot(snd_int->cyl), wr->end_shot(), mk_snd<object::ShotPtr>());
}

void Manager::shutdown()
{
	if (snd_int)
		snd_int->shutdown();
}

}
}



