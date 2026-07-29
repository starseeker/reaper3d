/* $Id: snd_system.cpp,v 1.68 2002/05/21 10:09:28 pstrand Exp $ */



#include <memory>
#include <string>

#include "hw/snd.h"
#include "hw/snd_wave.h"
#include "hw/snd_mp3.h"
#include "hw/snd_subsystem.h"

#include "res/config.h"
#include "res/res.h"
#include "hw/interfaces.h"
#include "res/resource.h"

#include "hw/debug.h"

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
};

using namespace res;


struct SoundSystem_impl
{
	float glob_volume;
	float ratio;

	Subsystem* music_player;
	Subsystem* sound_player;
	Main main;
	std::unique_ptr<Subsystem> dummy;
	std::unique_ptr<Subsystem> openal;

	SoundSystem_impl()
	 : glob_volume(0.5), ratio(0.0),
	   music_player(0), sound_player(0),
	   dummy(create_snd_dummy(&main)),
	   openal(create_snd_openal(&main))
	{
		dlog << "SoundSystem_impl enter\n";
		
		sound_player = music_player = openal.get();

		sound_player->set_volume(0.8);
		music_player->set_volume(0.3);
		
		dlog << "SoundSystem_impl done\n";

	}

	~SoundSystem_impl()
	{
		dlog << "~SoundSystem_impl enter\n";
		dlog << "~SoundSystem_impl done\n";
	}

	AudioSourcePtr load(res::ResourceClass type, const string& id)
	{
		AudioDecoderPtr decoder =
			type == res::Sound
				? AudioDecoderPtr(new WaveDecoder)
				: AudioDecoderPtr(new Mp3Decoder);
		decoder->init(std::make_unique<res::res_stream>(
			type, id, res::throw_on_error));
		return decoder->get();
	}
};


SoundSystem::SoundSystem()
 : impl(std::make_unique<SoundSystem_impl>())
{
}

SoundSystem::~SoundSystem() = default;


bool SoundSystem::init()
{
	if (!impl->sound_player->init()) {
		derr << "OpenAL device unavailable; falling back to silent audio\n";
		impl->openal.reset();
		impl->sound_player = impl->music_player = impl->dummy.get();
		impl->sound_player->init();
	}
	return true;
}

EffectPtr SoundSystem::prepare_effect(const string& id)
{

	EffectPtr eff;
	try {
		AudioSourcePtr source = impl->load(res::Sound, id);
		if (source)
			eff = impl->sound_player->prepare(std::move(source));
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
		AudioSourcePtr source = impl->load(res::Music, id);
		if (source)
			return impl->music_player->prepare_streaming(
				std::move(source));
	} catch (error_base& e) {
		derr << "failed to load music: " << e.what() << '\n';
		return std::unique_ptr<Sound>(dummysound().release());
	}
	return SoundPtr(nullptr);
}

EffectPtr SoundSystem::dummysound()
{
	return impl->dummy->prepare(nullptr);
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
