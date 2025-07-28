#ifndef REAPER_GAME_HELPERS_H
#define REAPER_GAME_HELPERS_H

#include <map>

#include "game/scenario_mgr.h"
#include "game/state.h"
#include "gfx/fps_meter.h"
#include "gfx/gfx.h"
#include "hw/reltime.h"
#include "hw/abstime.h"
#include "hw/profile.h"
#include "net/game_mgr.h"
#include "phys/engine.h"
#include "snd/sound_system.h"
#include "world/worldref.h"
#include "game/state.h"

#include "gfx/cloud/cloud.h"

namespace reaper {
namespace {
        typedef std::map<std::string, std::string> Args;
}

namespace game {

struct Timers {
	hw::time::Profiler2 phys, gfx, ai, wait, io, snd, world, net;
	hw::time::Profiler frame;
	Timers();

	void draw_txt();
	void draw_last(float frame_length);
	void draw_avg(float frame_length);
	void draw_history(float frame_length);

	void print_txt();
};

struct Config {
	std::string level;
	bool record;
	bool networked;
	bool server;
	bool observer;
	std::string playback;
	std::string scenario;
	bool fixed_dt;

	bool draw_timers;
	bool hud;
	int step_dt;
	bool render;
	bool print_timers;
	bool split_screen;
	std::string servername;
	std::vector<std::string> player_ships;

	std::string player_name;

	bool clouds;
	int max_frames;

	bool print_mtx;

	Config(const Args& args);

	void reload();
	void dump(state::Env&) const;
};


class ProgressBar;

struct Game : public state::Persistent
{
	Config conf;
	hw::gfx::Gfx& gx;
	
	gfx::RendererRef rr;

	world::WorldRef wr;
	reaper::sound::SoundRef sm;
	reaper::phys::PhysRef pr;
	game::scenario::ScenarioRef sr;
	object::PlayerPtr ply, ply2;
	net::GameMgr game;


	Timers timers;
	hw::time::Ticker loop_ticker;
	hw::event::EventProxy sys_ep;
	int cam;
	int frames;
	hw::time::RelTime sim_dt;

	int timescale;
	bool pause;
	bool draw_fps;

	gfx::FPSMeterVP fps;

	ProgressBar* loading;

	state::StateHolder sh;

	gfx::cloud::CloudSystem cloud_sys;

	hw::time::TimeSpan start_time;

	void apply_settings();
	
	Game(hw::gfx::Gfx& gx, const Args& args);
	~Game();

	void render_frame(double dt);

//	void create_player(const Point& p, const std::string& ship, int id);
	/// Create an insert usercontrolled players.
//	void create_players();

	/// Initialize game, ie: create various subsystems.
	void game_init();
	/// Prepare for startup, returns when ready.
	bool game_start();
	bool process_events();

	bool loop_once();
	void simulate(hw::time::RelTime time_ms, hw::time::RelTime diff_ms);

	void shutdown();

	gfx::Camera mk_cam(object::PlayerPtr, double tdiff);

	void render_stats();
	void print_stats();

	void dump(state::Env&) const;

	void restore(const std::string&);
};


}
}

#endif

