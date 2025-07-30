
#include "hw/compat.h"

#include <string>
#include <sstream>

#include "game/helpers.h"

#include "gfx/texture.h"
#include "gfx/matrix_utils.h"
#include "hw/gl.h"
#include "gfx/camera.h"
#include "gfx/misc.h"
#include "gfx/settings.h"
#include "hw/debug.h"
#include "hw/gfx.h"
#include "misc/font.h"
#include "misc/parse.h"
#include "misc/sequence.h"
#include "object/factory.h"
#include "object/object.h"
#include "res/config.h"
#include "res/res.h"
#include "res/resource.h"
#include "game/state.h"
#include "main/types_io.h"
#include "hw/worker.h"
#include "net/server.h"
#include "gfx/managers.h"
#include "gfx/abstracts.h"
#include "world/level.h"
#include "object/objectgroup.h"

#include "gfx/cloud/cloud.h"

#include "game/helpers.h"

#include <iomanip>
#include <iostream>


namespace reaper {
namespace object {
	int get_obj_count();
}
namespace game {

namespace {
	reaper::debug::DebugOutput derr("game::helpers");
	reaper::debug::DebugOutput dlog("game::helpers", 5);
}

namespace {
void setup_ortho() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
        glLoadIdentity();
	gfx::matrix_utils::ortho_2d(0, 1, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void restore_ortho() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

}

using namespace res;
using namespace misc;
using std::string;
using hw::time::RelTime;
using hw::time::get_rel_time;

Config::Config(const Args& args)
 : draw_timers(false)
{
	Args::const_iterator c, e = args.end();
	for (c = args.begin(); c != e; ++c) {
		stringpair a = split(c->second, ':');
		if (! (a.first.empty() || a.second.empty())) {
			stringpair b = split(a.second, '=');
			resource<ConfigEnv>(a.first)[b.first] = b.second;
		}
	}		

	reload();

}

void Config::reload()
{
	ConfigEnv cnf("game");
	string pfx = cnf["active"];
	if (!pfx.empty())
		pfx += '_';

	level     = withdefault(cnf, pfx + "level", std::string("test_level4"));
	record    = withdefault(cnf, pfx + "record", false);
	networked = withdefault(cnf, pfx + "networked", false);
	server    = withdefault(cnf, pfx + "server", false);
	observer  = withdefault(cnf, pfx + "observer", false);
	playback  = withdefault(cnf, pfx + "playback", std::string(""));
	scenario  = withdefault(cnf, pfx + "scenario", std::string(""));
	fixed_dt  = withdefault(cnf, pfx + "fixed_dt", false);
	hud       = withdefault(cnf, pfx + "hud", true);
	step_dt   = withdefault(cnf, pfx + "step_dt", 64);
	render    = withdefault(cnf, pfx + "render", true);
	print_timers = withdefault(cnf, pfx + "print_timers", !render);
	print_mtx = withdefault(cnf, pfx + "print_mtx", false);
	max_frames = withdefault(cnf, pfx + "max_frames", 0);

	split(withdefault(cnf, pfx+"player_ships", std::string("shipXP")),
			std::back_inserter(player_ships));

	split_screen = withdefault(cnf, pfx+"split_screen", false);
	player_name  = withdefault(cnf, pfx+"player_name", std::string("Player 1"));
	servername   = withdefault(cnf, pfx+"servername", std::string("localhost"));

	clouds    = withdefault(cnf, pfx + "clouds", false);
}

void Config::dump(state::Env& env) const
{
	env["fixed_dt"] = fixed_dt;
}

Timers::Timers()
 : phys("Phys ", 0, 15),
   gfx("Gfx  ", 0, 3),
   ai("AI   ", 0, 5),
   wait("Wait ", 0, 7),
   io("IO   ", 0, 9),
   snd("Snd  ", 0, 11),
   world("World", 0, 13),
   net("Net  ", 0, 1),
   frame("Frame ")
{ }

// Gah!

void Timers::draw_txt()
{
	phys.draw_txt();
	gfx.draw_txt();
	ai.draw_txt();
	wait.draw_txt();
	io.draw_txt();
	snd.draw_txt();
	world.draw_txt();
	net.draw_txt();
}

class Foo { } foo;

std::ostream& operator<<(std::ostream& os, const Foo&) {
	os << std::right << std::fixed << std::setprecision(0) << std::setw(5);
	return os;
}

void Timers::print_txt()
{
	derr << "phys: "  << foo << phys.get_last()  << " (" << foo << phys.get_avg() << ") ";
	derr << "gfx: "   << foo << gfx.get_last()   << " (" << foo << gfx.get_avg() << ") ";
	derr << "ai: "    << foo << ai.get_last()    << " (" << foo << ai.get_avg() << ") ";
	derr << "io: "    << foo << io.get_last()    << " (" << foo << io.get_avg() << ") ";
	derr << "snd: "   << foo << snd.get_last()   << " (" << foo << snd.get_avg() << ") ";
	derr << "world: " << foo << world.get_last() << " (" << foo << world.get_avg() << ") ";
	derr << "net: "   << foo << net.get_last()   << " (" << foo << net.get_avg() << ") ";
	derr << '\n';
}

void Timers::draw_last(float fl)
{
	phys.draw_last(fl);
	gfx.draw_last(fl);
	ai.draw_last(fl);
	wait.draw_last(fl);
	io.draw_last(fl);
	snd.draw_last(fl);
	world.draw_last(fl);
	net.draw_last(fl);
}

void Timers::draw_avg(float fl)
{
	phys.draw_avg(fl);
	gfx.draw_avg(fl);
	ai.draw_avg(fl);
	wait.draw_avg(fl);
	io.draw_avg(fl);
	snd.draw_avg(fl);
	world.draw_avg(fl);
	net.draw_avg(fl);
}

void Timers::draw_history(float fl)
{
	glPointSize(0.5);
	glColor3f(1,1,1);
	phys.draw_history(fl);
	glColor3f(1,0,0);
	gfx.draw_history(fl);
	glColor3f(1,1,1);
	ai.draw_history(fl);
	glColor3f(1,0,0);
	wait.draw_history(fl);
	glColor3f(1,1,1);
	io.draw_history(fl);
	glColor3f(1,0,0);
	snd.draw_history(fl);
	glColor3f(1,1,1);
	world.draw_history(fl);
	glColor3f(1,0,0);
	net.draw_history(fl);
}

class ProgressBar
{
	hw::gfx::Gfx& gx;
	std::string label;
	float val, add, lbl_w;
	gfx::texture::Texture back;
public:
	ProgressBar(hw::gfx::Gfx&, const std::string& lbl, int max_ticks);
	void render(const std::string& msg);
	void tick(const std::string& msg);
};



ProgressBar::ProgressBar(hw::gfx::Gfx& g, const std::string& lbl, int max_ticks)
 : gx(g), label(lbl), val(0), add(1.0 / max_ticks), lbl_w(label.size() * 0.04),
   back("reaper-logo")
{
	glViewport(0, 0, gx.current_mode().width, gx.current_mode().height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gfx::matrix_utils::ortho_2d(0, 1, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.0, 0.0, 0.0, 1.0);
}

void ProgressBar::render(const std::string& msg)
{
	back.use();
	glColor3f(1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(0.0, 1.0);
		glTexCoord2f(1, 0);
		glVertex2f(1.0, 1.0);
		glTexCoord2f(1, 1);
		glVertex2f(1.0, 0.0);
		glTexCoord2f(0, 1);
		glVertex2f(0.0, 0.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	const float meter_y = 0.05;
	glColor3f(0.2, 0.2, 0.8);
	gfx::misc::meter(0.1, meter_y, 0.8, 0.05, 1.0, val);
	glColor3f(0.2, 0.6, 0.6);
	glBegin(GL_LINE_LOOP);
		glVertex2f(0.1, meter_y);
		glVertex2f(0.9, meter_y);
		glVertex2f(0.9, meter_y + 0.05);
		glVertex2f(0.1, meter_y + 0.05);
	glEnd();

	float pos = msg.length() * 0.01;
	gfx::font::glPutStr(0.5 - pos, meter_y + 0.01, msg, gfx::font::Medium, 0.02, 0.04);
}

void ProgressBar::tick(const std::string& msg)
{
	dlog << "progress: " << msg << '\n';
	val += add;
	render(msg);
	gx.update_screen();
}

void fade(float f)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	StateKeeper s1(GL_BLEND, true);
	glColor4f(0,0,0,f);
	glRectf(0,0,1,1);
}

Game::Game(hw::gfx::Gfx& g, const Args& args)
 : conf(args),
   gx(g),
   game(gx),
   sys_ep(hw::event::EventSystem::get_ref(hw::event::System)),
   cam(2), frames(0), sim_dt(0), timescale(1000), pause(false), draw_fps(false),
   loading(new ProgressBar(gx, "Loading: ", 10)),
   sh("game", this)
{
	rr = gfx::RendererRef::create();
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT);

	loading->render("init");
	if (gx.is_accelerated()) {
		for (float f = 1.0; f >= 0; f -= 0.04) {
			loading->render("init");
			fade(f);
			gx.update_screen();
		}
	}

	if (! gx.is_accelerated()) {
		rr->settings().texture_level = 0;
		rr->settings().use_fog = false;
		rr->settings().draw_sky = false;
	}
	loading->tick("init");
}


Game::~Game()
{
	game::scenario::ScenarioRef::destroy();
	sound::SoundRef::destroy();
	phys::PhysRef::destroy();
	world::WorldRef::destroy();
	gfx::RendererRef::destroy();
}


void Game::render_stats()
{
	using gfx::font::glPutStr;
	float frame_length = 1e6/50.0;

	StateKeeper s2(GL_DEPTH_TEST, false);
	StateKeeper s3(GL_LIGHTING, false);

	glColor4f(1,1,1,.5);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gfx::matrix_utils::ortho_2d(0,1,0,90);

	glMatrixMode(GL_MODELVIEW);

	char buf[100];
	snprintf(buf, 100,
		 "si: %3d st: %3d dyn: %3d shot: %3d  pairs: %4d  objs: %3d",
		 wr->silly_size(), wr->static_size(), 
		 wr->dynamic_size(), wr->shot_size(), pr->size(), object::get_obj_count());
	glPutStr(0, 17, buf, gfx::font::Medium, .008, 1.9);

	timers.draw_txt();

	StateKeeper s1(GL_TEXTURE_2D,false);
	glColor4f(0,0,1,.5);
	timers.draw_last(frame_length);

	glColor4f(0,1,0,.5);
	timers.draw_avg(frame_length);

	timers.draw_history(frame_length);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void Game::print_stats()
{
	char buf[100];
	snprintf(buf, 100,
		 "si: %3d st: %3d dyn: %3d shot: %3d  pairs: %4d  objs: %3d",
		 wr->silly_size(), wr->static_size(), 
		 wr->dynamic_size(), wr->shot_size(), pr->size(), object::get_obj_count());
	derr << buf << ' ';
	timers.print_txt();
}


void Game::apply_settings()
{
	conf.reload();
	rr->settings().hud_type = conf.hud ? gfx::External_HUD : gfx::None_HUD;
}

using hw::time::Profile;


class PlayerObjectSet : public NodeConfig<object::ObjectGroup>
{
public:
	typedef tp<object::ObjectGroup>::ptr Ptr;
	Ptr create(IdentRef id) {
		return resource_ptr<object::ObjectGroup>(id);

	}

};


void wait_net_display(const net::Players& ply, bool is_host)
{
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.5,0,0);
	gfx::font::glPutStr(0.1, 0.6, "Waiting for players to join", gfx::font::Medium, 0.02, 0.04);
	std::ostringstream msg;
	msg << "Joined: " << ply.size();
	gfx::font::glPutStr(0.1, 0.4, msg.str().c_str(), gfx::font::Medium, 0.02, 0.04);
	if (is_host) {
		gfx::font::glPutStr(0.1, 0.2, "Press enter to start", gfx::font::Medium, 0.02, 0.04);
	}
}

gfx::Camera Game::mk_cam(object::PlayerPtr ship, double tdiff)
{
	Point ship_pos = ship->get_pos();

	float speed = length(ship->get_velocity());
	Vector org(0, 0, -1);
	Vector dir(ship->get_mtx() * org);
	if (speed > 1.0)
		dir *= speed;

	Point c_pos;
	Point c_at = ship_pos;
	Vector c_up(0,1,0);
	int up_sign = dot(Vector(0,1,0), ship->get_mtx().col(1)) >= 0 ? 1 : -1;

	switch (cam) {
	case 1:	c_pos = ship_pos - dir/30 - norm(dir) * 15 + Point(0,2,0);
		c_up = Vector(0, up_sign, 0);
		break;
	case 2:
		c_at = ship_pos + norm(dir)*20;
		c_pos = ship_pos - dir/30 - norm(dir) * 15 + Point(0,2,0);
		c_up = Vector(0, up_sign, 0);
		break;
	case 3:
		c_pos = ship_pos - ship->get_mtx().col(2);
		c_at  = c_pos - ship->get_mtx().col(2);
		c_up = norm(ship->get_mtx().col(1));
		break;
	case 4: c_pos = ship_pos - Vector(0, -10, 10);break;
	case 5: c_pos = ship_pos - 10*norm(dir);  break;
	case 6: c_pos = ship_pos - Vector(0, 0, 40);break;
	case 7: c_pos = ship_pos - Vector(0, 60, 60);break;
	default: c_pos = ship_pos - Vector(0, -160, 80);
	}

	if(ship->is_dead()) {
		static float angle = 0;
		angle += tdiff/3;
		c_at = ship_pos;
		c_pos = c_at + Vector(sin(angle)*30,20,cos(angle)*30);
		float alt = wr->get_altitude(Point2D(c_pos.x, c_pos.z));
		c_pos.y = min(c_pos.y, alt+2);
	}

	return gfx::Camera(c_pos, c_at, c_up, 100, 75);
}

typedef std::vector<std::string> strvec;

class ObjGrpOverride : public NodeConfig<object::ObjectGroup>
{
	strvec ships;
	bool found;

	class Mod {
		const strvec& vec;
		bool& found;
	public:
		Mod(const strvec& v, bool& f) : vec(v), found(f) { }
		object::MkInfo operator()(object::MkInfo mk) {
			if (mk.id > 0 && mk.id <= vec.size()) {
				mk.name = vec[mk.id-1];
				found = true;
			}
			return mk;
		}
	};
public:
	ObjGrpOverride(const strvec& v) : ships(v), found(false) { }

	typedef tp<ObjectGroup>::ptr Ptr;
	Ptr create(IdentRef id) {
		Ptr p = parent()->create(id);
		if (!found) {
			transform(seq(p->objs), p->objs.begin(), Mod(ships, found));
		}
		return p;
	}

};

void load_mesh(const string& mesh)
{
	dlog << "preload mesh: " << mesh << '\n';
	gfx::MeshRef()->load(mesh);
}

void load_tex(const string& tex)
{
	dlog << "preload texture: " << tex << '\n';
	gfx::TextureRef()->load(tex);
}


void Game::game_init()
{
	if (conf.server)
		game.start_server();
	loading->tick("world");
	{ Profile p("world");
	  wr = world::World::create();
	  wr->load(conf.level);
	}
	loading->tick("phys");
	{ Profile p("phys");
	  pr = phys::Engine::create();
	}

	if (! conf.playback.empty())
		game.playback_game(conf.playback);
	else if (conf.record)
		game.record_game(true);

	loading->tick("level");
	{ Profile p("rend");
	  rr->load();
	  rr->settings().hud_type = gfx::External_HUD;
	  rr->settings().radar_range = 1600;
	}

	loading->tick("settings");
	{ Profile p("rest");
	  apply_settings();
	}

	loading->tick("sounds");
	{ Profile p("snd");
	  sm = sound::Manager::create();
	}

	loading->tick("scenario");
	{ Profile scp("scen_mgr");
	  std::string sc = conf.scenario;
	  if (sc.empty())
	  	sc = wr->get_level_info().scenario;
	  if (sc.empty())
	  	sc = "test_level4";
	  sr = game::scenario::ScenarioRef::create();
	  push_config<object::ObjectGroup>(new ObjGrpOverride(conf.player_ships));
	  sr->init(sc);
	  pop_config<object::ObjectGroup>();
	}

	loading->tick("objects");
	{ Profile p("objects");
	  gfx::MeshRef mr;
	  gfx::TextureRef tr;

	  set<string> obj_names;
	  sr->get_objectnames(obj_names);

	  set<string>::iterator c, e = obj_names.end();
	  set<string> meshes, textures;
	  for (c = obj_names.begin(); c != e; ++c) {
		string mesh = withdefault(object::factory::inst().info(*c), string("mesh"), *c);
		misc::Unique tex = mr->get_texture(gfx::RenderInfo(mesh, Matrix::id(), false));
		meshes.insert(mesh);
		textures.insert(tex.get_text());
	  }
	  for_each(seq(meshes), load_mesh);
	  for_each(seq(textures), load_tex);
	}
}

bool Game::game_start()
{
	loading->tick("startup");

	object::ID local_id = 1;
	{ Profile p("init");
	  game.init(conf.networked, conf.servername, conf.split_screen);

	  local_id = game.alloc_id(conf.observer);
	}

	sr->update(0.01);

	pr->update_world(-0.01, 0.01);
	wr->set_local_player(local_id);
	ply = wr->get_local_player();

	if (conf.split_screen)
		ply2.dynamic_assign(wr->lookup_dyn(game.alloc_id(false)));


	loading->tick("done");
	if (gx.is_accelerated()) {
		for(float f = 0; f < 1.0; f += 0.01) {
			loading->render("done");
			fade(f);
			gx.update_screen();
		}
	}

	bool server_wait_start = true;
	if (conf.networked) {
		hw::event::Event junk;
		while (sys_ep.get_event(junk))
			;
		while (!game.get_multistatus() && server_wait_start) {
			wait_net_display(game.players(), conf.server);
			if (conf.server) {
				hw::event::Event e;
				while (sys_ep.get_event(e)) {
					using namespace hw::event;
					if (e.id == id::Enter) {
						server_wait_start = false;
						break;
					} else if (e.id == id::Escape) {
						return false;
					}
				}
			}
			gx.update_screen();
			hw::time::msleep(100);
		}
	}

	if (!game.start_req())
		derr << "Start failed!\n";

	const net::Players& plys = game.players();
	for (int i = 1; i < 20; ++i) {
		if (plys.find(i) == plys.end()) {
			pr->remove(i);
			object::DynamicPtr d = wr->lookup_dyn(i);
			if (d.valid())
				wr->erase(d->get_id());
		} else {
			game.add_sync(wr->lookup_dyn(i));
		}
	}

	if (conf.clouds)
		cloud_sys.init();
//	gx.update_screen();
//	hw::time::msleep(2000);

	rr->start();

	render_frame(0.0);

	game.go_go_go();
	loop_ticker.reset();
	start_time = hw::time::get_time();

	timers.frame.start();

	sim_dt = 0;
//	derr << "Start: " << game.ticker.last().approx().to_s() << '\n';
	delete loading;
	loading = 0;
	return true;
}

bool inv_bool(bool& b)
{
	b = !b;
	return b;
}

void inc_mod(int& v, int m)
{
	++v;
	v %= m;
}

void set_in_range(float& f, float min, float max)
{
	if (f < min)
		f = min;
	else if (f > max)
		f = max;
}

void nonlin_add(int &v, int s)
{
	if (v < 10 && s < 0)
		return;
	if (v < 500)
		v += s/10;
	else
		v += s;
}

bool Game::process_events()
{
	namespace id = hw::event::id;
	hw::event::Event e;
	if (sys_ep.get_event(e)) {
		if (e.id >= id::Btn0 || e.val < 0.1)
			return false;

		switch (e.id) {
		case id::Escape: return true;
		case id::F1: cam = 1; rr->settings().hud_type = gfx::None_HUD; break;
		case id::F2: cam = 2; rr->settings().hud_type = gfx::External_HUD; break;
		case id::F3: cam = 3; rr->settings().hud_type = gfx::Internal_HUD; break;
		case id::F4: cam = 4; rr->settings().hud_type = gfx::None_HUD; break;
		case id::F5: cam = 5; rr->settings().hud_type = gfx::None_HUD; break;
		case id::F6: cam = 6; rr->settings().hud_type = gfx::None_HUD; break;
		case id::F7: cam = 7; rr->settings().hud_type = gfx::None_HUD; break;
		case id::F8: cam = 8; rr->settings().hud_type = gfx::None_HUD; break;
		case '6': inc_mod(rr->settings().texture_level, 3);
			  break;
		case '7': inv_bool(rr->settings().use_lighting);
 			  break;
		case '8': inv_bool(rr->settings().draw_sky);
			  break;
		case '9': inv_bool(rr->settings().draw_terrain);
			  break;
		case '0': inv_bool(rr->settings().use_fog);
			  break;
		case 'Y': (++rr->settings().shadow_type) %= 6;
			  break;
		case 'U': inv_bool(rr->settings().draw_effects);
			  break;
		case 'I': rr->screenshot();
			  break;

		case 'W': { 	static int s = 1;

				conf.draw_timers = (s != 0);
				rr->settings().draw_stats = s-1;
				++s %= 6;
			}
			break;

		case 'R': {	static int r = 5;
				rr->settings().radar_range = 100 * (1 << r); 
				++r &= 7;
			}
			break;
		case 'S': rr->settings().terrain_detail -= 0.02;
		case 'X': rr->settings().terrain_detail += 0.01;
			  set_in_range(rr->settings().terrain_detail, 0.05, 1);
			  derr << "detail: " << rr->settings().terrain_detail << '\n';
			  break;
	  	case 'D': rr->settings().texture_scaling <<= 1;
			  if (rr->settings().texture_scaling > 16)
				  rr->settings().texture_scaling = 1;
			  rr->purge_textures();
			  derr << "texture scaled down to 1/" << rr->settings().texture_scaling << " th\n";
			  break;
		case 'F': inv_bool(draw_fps);
			  fps.reinit();
			  break;
		case 'T':
		case 'G':
			nonlin_add(timescale, (e.id == 'T') ? 50 : -50);
			derr << "Time scaled by: " << std::fixed << std::setw(5)
			     << std::setprecision(3) << timescale / 1000.0 << '\n';
			break;
		case 'P': inv_bool(pause); loop_ticker.pause(pause); break;
		case id::F11: {
			  state::statemgr().dump("latest");
			  break;
			}
		case id::F12:
			return true;
		default: break;
		}
	}
	return false;
}



void Game::restore(const std::string&)
{
}

void report_stuff(object::PlayerPtr p)
{
	derr << "time: " << get_rel_time() << ' ' << p->get_mtx() << '\n';
}

bool Game::loop_once()
{
	++frames;

	loop_ticker.tick(timescale / 1000.0);

	RelTime real_diff_ms = loop_ticker.last_ms();
	sim_dt += real_diff_ms;

	RelTime now_ms = get_rel_time();
	RelTime diff_ms = real_diff_ms;

	{ Profile p(timers.io);
		if (process_events()) {
			return true;
		}
	}
	if (conf.max_frames > 0 && frames >= conf.max_frames)
		return true;

	if (!pause) {
		if (conf.fixed_dt) {
			RelTime step_ms = conf.step_dt;
			while (sim_dt > step_ms) {
				simulate(now_ms, step_ms);
				{ Profile(timers.net);
					game.update();
				}
//				if ((get_rel_time() % 1000) == 0)
//					report_stuff(wr->get_local_player());
				sim_dt -= step_ms;
				now_ms += step_ms;
			}
		} else {
			const RelTime max_dt = 50;

			RelTime time_ms = now_ms;
			RelTime delta_time_ms = diff_ms;

			while (delta_time_ms > max_dt) {
				simulate(time_ms, max_dt);
				delta_time_ms -= max_dt;
				time_ms += max_dt;
			}

			simulate(now_ms, delta_time_ms);
			sim_dt = 0;
			{ Profile(timers.net);
				game.update();
			}
		}

		{ Profile(timers.world);
			wr->update();
		}
	}

	{ Profile p(timers.wait);
	  gx.update_screen();
	}

	timers.frame.stop();
	timers.frame.start();
	
//	cloud_sys.simul();
//	  gx.update_screen();
//	  hw::time::msleep(300);
	render_frame(diff_ms / 1000.0);

	if (draw_fps)
		fps.render();

	glDisable(GL_DEPTH_TEST);

	if (conf.draw_timers)
		render_stats();
	if (conf.print_timers)
		print_stats();
	if (conf.print_mtx && frames%50 == 0) {
		derr << wr->get_local_player()->get_mtx() << '\n';
	}
	sr->draw_info();

	if (timescale != 1000.0) {
		stringstream ss;
		ss << "Time scaled by: " << std::fixed << std::setw(5)
		   << std::setprecision(3) << timescale / 1000.0 << " - " << (now_ms/1000.0);
		
		setup_ortho();
		gfx::font::glPutStr(0.5, 0.9, ss.str().c_str());
		restore_ortho();

	}

	if (!pause) {
		if ((frames % 5) == 0) {
			Profile p(timers.snd);
			sm->set_camera(ply->get_pos(), ply->get_velocity(), ply->get_velocity());
			sm->run();
		}
	}
	return false;
}

void render_viewport(gfx::Camera cm, int x, int y, int w, int h, PlayerPtr ply)
{
	cm.horiz_fov /= 2;
	glViewport(0, 0, w, h);
	glScissor(0, 0, w, h);
	world::WorldRef()->set_local_player(ply->get_id());
	gfx::RendererRef()->render(cm);
}

void Game::render_frame(double diff_s)
{
	{ Profile p(timers.gfx);
	  if (conf.render) {
		if (conf.split_screen) {
			glEnable(GL_SCISSOR_TEST);
			hw::gfx::VideoMode vm = gx.current_mode();

			render_viewport(mk_cam(ply, diff_s), 0, 0, vm.width/2, vm.height, ply);
			render_viewport(mk_cam(ply2, diff_s), vm.width/2, 0, vm.width/2, vm.height, ply2);

			wr->set_local_player(ply->get_id());
			glViewport(0, 0, vm.width, vm.height);
			glScissor(0, 0, vm.width, vm.height);
			glDisable(GL_SCISSOR_TEST);
		} else {
			const gfx::Camera& cam = mk_cam(ply, diff_s);
			if (conf.clouds)
				cloud_sys.update(cam, ply->get_mtx());
			rr->render(cam);
		}
	  }
	}
}

void Game::shutdown()
{
	derr << "Shutdown game\n";
	game.shutdown();
	if (sm.valid())
		sm->shutdown();
	if (sr.valid())
		sr->shutdown();
	if (pr.valid())
		pr->shutdown();
	wr->shutdown();
	ply.invalidate();
	ply2.invalidate();
}

void Game::dump(state::Env& env) const
{
	conf.dump(env);
}

void Game::simulate(RelTime time_ms, RelTime dt_ms)
{
	double now_s = time_ms / 1000.0;
	double dt_s  = dt_ms / 1000.0;
	{ Profile p(timers.ai);
		sr->update(dt_s);
	}
	{ Profile p(timers.phys);
		pr->update_world(now_s, dt_s);
	}
	hw::time::rel_time_tick(dt_ms);
	rr->simulate(dt_s);
}


}
}

