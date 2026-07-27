/* $Id: snd_system.cpp,v 1.68 2002/05/21 10:09:28 pstrand Exp $ */


#include "hw/compat.h"

#include <deque>
#include <map>
#include <functional>
#include <string>
#include <memory>

#include "hw/snd.h"
#include "hw/snd_wave.h"
#include "hw/snd_mp3.h"
#include "hw/snd_subsystem.h"

#include "res/config.h"
#include "res/res.h"
#include "misc/free.h"
#include "misc/sequence.h"
#include "hw/interfaces.h"
#include "world/world.h"
#include "res/resource.h"

#include "hw/debug.h"
#include "hw/worker.h"

// Backend factories are kept as direct symbols while the old plugin loader is
// removed from the monolithic build.
extern "C" reaper::hw::snd::Subsystem* create_snd_dummy(void*);
extern "C" reaper::hw::snd::Subsystem* create_snd_openal(void*);

namespace reaper {
namespace hw {
namespace snd {

namespace { 
	debug::DebugOutput derr("snd_subsystem", 0);
	debug::DebugOutput dlog("snd_subsystem", 5);
}

using misc::seq;
using misc::for_each;
using misc::delete_it;

using std::string;


class Main
 : public ifs::Snd
{
	debug::DebugOutput dr;
public:
	Main() : dr("snd_plugin", 5)
	{ }
	debug::DebugOutput& derr() { return dr; }
	std::string config(const std::string& s) {
		res::ConfigEnv env("hw_snd_system");
		return env[s];
	}
	void add_job(worker::Job* j) {
		worker::worker()->add_job(j, false);
	}
};

using namespace res;


class AudioDecoderCreator : public NodeConfig<AudioDecoder>
{
	typedef tp<AudioDecoder>::ptr Ptr;

public:
	
	Ptr create(IdentRef id) {
		return Ptr(0);
	}
};


struct SoundSystem_impl
{
	float glob_volume;
	float ratio;

	Subsystem* music_player;
	Subsystem* sound_player;
	Subsystem* dummy;
	Subsystem* openal;

	std::map<string, AudioDecoder*> decoders;

	Main main;

	SoundSystem_impl()
	 : glob_volume(0.5), ratio(0.0),
	   music_player(0), sound_player(0),
	   dummy(0), openal(0)
	{
		dlog << "SoundSystem_impl enter\n";
		
		dummy = create_snd_dummy(&main);
		openal = create_snd_openal(&main);
		sound_player = music_player = openal;

		sound_player->set_volume(0.8);
		music_player->set_volume(0.3);
		
		dlog << "SoundSystem_impl done\n";

//		res::push_config<AudioDecoder>(new AudioDecoderCreator(), "audio");
	}

	~SoundSystem_impl()
	{
		dlog << "~SoundSystem_impl enter\n";
		for_each(seq(decoders), delete_it);

		if (sound_player == music_player)
			delete sound_player;
		else {
			delete sound_player;
			delete music_player;
		}
		if (dummy != sound_player && dummy != music_player)
			delete dummy;

//		res::pop_config<AudioDecoder>();
		dlog << "~SoundSystem_impl done\n";
	}

	AudioSourcePtr load(res::ResourceClass type, const string& id)
	{
		AudioDecoder* decoder = type == res::Sound
			? static_cast<AudioDecoder*>(new WaveDecoder)
			: static_cast<AudioDecoder*>(new Mp3Decoder);
		try {
			decoder->init(new res::res_stream(type, id, res::throw_on_error));
			AudioSourcePtr source = decoder->get();
			delete decoder;
			return source;
		} catch (...) {
			delete decoder;
			throw;
		}
	}
};


SoundSystem::SoundSystem()
 : impl(new SoundSystem_impl())
{
}

SoundSystem::~SoundSystem()
{
	delete impl;
}


bool SoundSystem::init()
{
	if (!impl->sound_player->init()) {
		derr << "OpenAL device unavailable; falling back to silent audio\n";
		delete impl->openal;
		impl->openal = nullptr;
		impl->sound_player = impl->music_player = impl->dummy;
		impl->sound_player->init();
	}
	return true;
}

EffectPtr SoundSystem::prepare_effect(const string& id)
{

	EffectPtr eff;
	try {
		if (AudioSource* src = impl->load(res::Sound, id))
			eff = impl->sound_player->prepare(src);
	} catch (error_base& h) {
		derr << "sound preparation error: " << h.what() << '\n';
	}
	if (eff.get())
		return eff;
	else
		return dummysound();
}

SoundPtr SoundSystem::prepare_music(const string& id)
{
	try {
		if (AudioSource* src = impl->load(res::Music, id))
			return impl->music_player->prepare_streaming(src);
	} catch (error_base& e) {
		derr << "failed to load music: " << e.what() << '\n';
		return std::unique_ptr<Sound>(dummysound().release());
	}
	return SoundPtr(nullptr);
}

EffectPtr SoundSystem::dummysound()
{
	return impl->dummy->prepare((AudioSourcePtr)0);
}

void SoundSystem::set_volume(float vol)
{
	if (impl->ratio < 0) {
		impl->sound_player->set_volume(vol * (1 + impl->ratio));
		impl->music_player->set_volume(vol);
	} else {
		impl->sound_player->set_volume(vol);
		impl->music_player->set_volume(vol * (1 - impl->ratio));
	}
	impl->glob_volume = vol;
}

void SoundSystem::set_listener(const Point& pos, const Vector& dir, const Vector& vel)
{
	impl->sound_player->set_listener(pos, dir, vel);
}


void SoundSystem::do_stuff()
{
	if (impl->music_player)
		impl->music_player->do_stuff();
}



}
}
}
