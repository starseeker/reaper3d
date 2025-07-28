
#include "hw/compat.h"

#include <iostream>
#include <deque>

#include "hw/gfx.h"
#include "hw/event.h"
#include "hw/time.h"
#include "hw/mapping.h"
#include "hw/snd.h"
#include "hw/video.h"

#include "misc/menu.h"
#include "misc/font.h"
#include "misc/test_main.h"
#include "misc/sequence.h"
#include "misc/free.h"

#include "game/helpers.h"

#include "gfx/abstracts.h"
#include "gfx/settings.h"

#include "res/res.h"
#include "res/config.h"

#include <string>
#include <sstream>

namespace reaper {
using namespace menu;
using namespace std;

namespace {
inline pair<string, string> make_strp(const string &s1, const string &s2) {
	return make_pair<string, string>(s1,s2);
}
}


using hw::event::EventSystem;
using hw::event::EventProxy;

struct Resolution {
	int w, h;
	Resolution(int W, int H) : w(W), h(H) {}
	bool operator==(const Resolution &r) const {
		return w == r.w && h == r.h;
	}
	bool operator!=(const Resolution &r) const {
		return !operator==(r);
	}
};

typedef vector<pair<string, Resolution> > ResolutionCont;
typedef vector<pair<string, string> > StringCont;

typedef MenuItem<int> BaseItem;
typedef MenuItemLabel<int> Label;
typedef MenuItemRetVal<int> Item;
typedef MenuItemStringBox<int> StringBox;
typedef MenuItemTwoChoice<int> Choice;
typedef MenuItemMultiChoice<int, ResolutionCont::iterator, Resolution> ResChoice;
typedef MenuItemMultiChoice<int, StringCont::iterator, string> StringChoice;

typedef deque<BaseItem*> MenuCont;

typedef Menuexec<MenuCont::iterator, int> MenuE;

class Menu  {
	deque<Resolution> resolutions;

	MenuCont main_menu;
	MenuCont single_menu;
	MenuCont multi_menu;
	MenuCont options_menu;
	MenuCont host_menu;
	MenuCont join_menu;
	MenuCont video_menu;
	MenuCont audio_menu;
	MenuCont controls_menu;

	ResolutionCont res_choices;
	StringCont audio_devs;
	StringCont openal_context;

	// Menu-settable variables
	string player_name;
	string server_name;

	gfx::Settings gfx_sets;
	bool fullscreen;
	Resolution resolution;

	string sound_dev, music_dev, openal;
	
	bool use_joystick, joystick_debug;
	string joystick;

	// Blah...
	hw::gfx::Gfx gx;
	EventSystem es;
	hw::snd::SoundSystem sm;
	deque<Texture*> back;

	bool singleplayer();

	bool multiplayer();
	bool join_multiplayer();
	bool host_multiplayer();

	void options();
	void audio_options();
	void video_options();
	void control_options();

	void add_resolution(const Resolution&);
public:
	Menu();
	~Menu();

	bool main();
};

Menu::Menu() : player_name("Rookie One"), server_name("ReaperServer"), 
               fullscreen(false), resolution(640,480), gx(), es(gx), sm()	       
{
//	sm.init();
	gfx::Settings::current = gfx::Settings::defaults;

	back.push_back(new Texture("menu_bkg_1"));
	back.push_back(new Texture("menu_bkg_2"));
	back.push_back(new Texture("menu_bkg_3"));
	back.push_back(new Texture("menu_bkg_4"));

	resolutions.push_back(Resolution(320, 200));
	resolutions.push_back(Resolution(400, 300));
	resolutions.push_back(Resolution(640, 480));
	resolutions.push_back(Resolution(800, 600));
	resolutions.push_back(Resolution(1024, 768));
	resolutions.push_back(Resolution(1280, 960));
	resolutions.push_back(Resolution(1280, 1024));
	resolutions.push_back(Resolution(1600, 1200));

	audio_devs.push_back(make_strp("Disabled", "dummy"));
	audio_devs.push_back(make_strp("Simple", "simple"));
	audio_devs.push_back(make_strp("OpenAL", "openal"));
	audio_devs.push_back(make_strp("DSound", "dsound"));

	openal_context.push_back(make_strp("Wave", "wave"));
	openal_context.push_back(make_strp("DSound", "DirectSound"));
	openal_context.push_back(make_strp("DSound3D", "DirectSound3D"));

	// Setup resolution stuff
	for(deque<Resolution>::const_iterator i = resolutions.begin(); i != resolutions.end(); ++i) {
		add_resolution(*i);
	}

	res::ConfigEnv hw_gfx("hw_gfx");
	resolution.w = res::withdefault(hw_gfx, "width", 640);
	resolution.h = res::withdefault(hw_gfx, "height", 480);
	fullscreen   = res::withdefault(hw_gfx, "fullscreen", true);

	// If a special resolution is entered manually into hw_gfx settings file,
	// add it to the list of possible choices
	ResolutionCont::iterator i;
	for(i = res_choices.begin(); i != res_choices.end(); ++i) {
		if(i->second.w == resolution.w && i->second.h == resolution.h) {
			break;
		}
	}
	if(i == res_choices.end()) {
		add_resolution(resolution);
	}

	// Setup audio stuff
	res::ConfigEnv hw_snd_system("hw_snd_system");
	sound_dev = res::withdefault(hw_snd_system, "sound", "simple");
	music_dev = res::withdefault(hw_snd_system, "music", "simple");
	openal    = res::withdefault(hw_snd_system, "openal_context", "Wave");

	// Setup controls stuff
	res::ConfigEnv hw_input("hw_input");
	use_joystick = res::withdefault(hw_input, "use_joystick", false);
	joystick     = res::withdefault(hw_input, "joystick", "hw_joystick_standard");
	joystick_debug = res::withdefault(hw_input, "joystick_debug", false);

	// Init menus
	main_menu.push_back(new Item(Box(0.1, 0.1, 0.4, 0.2), "Singleplayer", 1));
	main_menu.push_back(new Item(Box(0.1, 0.3, 0.4, 0.4), "Multiplayer", 2));
	main_menu.push_back(new Item(Box(0.1, 0.5, 0.4, 0.6), "Options", 3));
	main_menu.push_back(new Item(Box(0.1, 0.7, 0.4, 0.8), "Movie", 4));
	main_menu.push_back(new Item(Box(0.8, 0.8, 0.95, 0.9), "Quit", 0));

	single_menu.push_back(new Label(Box(0.1, 0.1, 0.4, 0.2), "Player Name"));
	single_menu.push_back(new StringBox(Box(0.1, 0.2, 0.4, 0.3), &player_name));
	single_menu.push_back(new Item(Box(0.1, 0.5, 0.4, 0.6), "Start", 2));
	single_menu.push_back(new Item(Box(0.8, 0.8, 0.95, 0.9), "Back", 0));

	multi_menu.push_back(new Item(Box(0.1, 0.1, 0.4, 0.2), "Host Game", 1));
	multi_menu.push_back(new Item(Box(0.1, 0.3, 0.4, 0.4), "Join Game", 2));
	multi_menu.push_back(new Item(Box(0.8, 0.8, 0.95, 0.9), "Back", 0));

	options_menu.push_back(new Item(Box(0.1, 0.1, 0.4, 0.2), "Video", 1));
	options_menu.push_back(new Item(Box(0.1, 0.3, 0.4, 0.4), "Audio", 2));
	options_menu.push_back(new Item(Box(0.1, 0.5, 0.4, 0.6), "Controls", 3));
	options_menu.push_back(new Item(Box(0.8, 0.8, 0.95, 0.9), "Back", 0));

	host_menu.push_back(new Label(Box(0.1, 0.1, 0.4, 0.2), "Player Name"));
	host_menu.push_back(new StringBox(Box(0.1, 0.2, 0.4, 0.3), &player_name));
	host_menu.push_back(new Label(Box(0.1, 0.3, 0.4, 0.4), "Server Name"));
	host_menu.push_back(new StringBox(Box(0.1, 0.4, 0.4, 0.5), &server_name));
	host_menu.push_back(new Item(Box(0.1, 0.5, 0.4, 0.6), "Choose Ship", 3));
	host_menu.push_back(new Item(Box(0.1, 0.7, 0.4, 0.8), "Choose Team", 4));
	host_menu.push_back(new Item(Box(0.1, 0.9, 0.4, 1.0), "Launch Game", 5));
	host_menu.push_back(new Item(Box(0.8, 0.8, 0.95, 0.9), "Back", 0));

	join_menu.push_back(new Label(Box(0.1, 0.1, 0.4, 0.2), "Player Name"));
	join_menu.push_back(new StringBox(Box(0.1, 0.2, 0.4, 0.3), &player_name));
	join_menu.push_back(new Label(Box(0.1, 0.3, 0.4, 0.4), "Join Server"));
	join_menu.push_back(new StringBox(Box(0.1, 0.4, 0.4, 0.5), &server_name));
	join_menu.push_back(new Item(Box(0.1, 0.5, 0.4, 0.6), "Choose Ship", 3));
	join_menu.push_back(new Item(Box(0.1, 0.7, 0.4, 0.8), "Choose Team", 4));
	join_menu.push_back(new Item(Box(0.1, 0.9, 0.4, 1.0), "Join Game", 5));
	join_menu.push_back(new Item(Box(0.8, 0.8, 0.95, 0.9), "Back", 0));

	video_menu.push_back(new ResChoice(Box(0.1, 0.1, 0.4, 0.2), res_choices.begin(), res_choices.end(), &resolution));
	video_menu.push_back(new Choice(Box(0.1, 0.2, 0.4, 0.3), &fullscreen, "Fullscreen", "Windowed"));
	video_menu.push_back(new Choice(Box(0.1, 0.3, 0.4, 0.4), &gfx_sets.use_texturing, "Texturing on", "Texturing off"));
	video_menu.push_back(new Choice(Box(0.1, 0.4, 0.4, 0.5), &gfx_sets.use_lighting, "Lighting on", "Lighting off"));
	video_menu.push_back(new Choice(Box(0.1, 0.5, 0.4, 0.6), &gfx_sets.use_fog, "Fog on", "Fog off"));
	video_menu.push_back(new Choice(Box(0.1, 0.6, 0.4, 0.7), &gfx_sets.draw_shadows, "Shadows on", "Shadows off"));
	video_menu.push_back(new Choice(Box(0.1, 0.7, 0.4, 0.8), &gfx_sets.draw_lights, "Lights on", "Lights off"));

	video_menu.push_back(new Choice(Box(0.5, 0.1, 0.8, 0.2), &gfx_sets.draw_terrain, "Terrain on", "Terrain off"));
	video_menu.push_back(new Choice(Box(0.5, 0.2, 0.8, 0.3), &gfx_sets.draw_effects, "Effects on", "Effects off"));
	video_menu.push_back(new Choice(Box(0.5, 0.3, 0.8, 0.4), &gfx_sets.draw_sky, "Sky on", "Sky off"));
	video_menu.push_back(new Choice(Box(0.5, 0.4, 0.8, 0.5), &gfx_sets.draw_water, "Water on", "Water off"));

	video_menu.push_back(new Item(Box(0.6, 0.8, 0.75, 0.9), "Cancel", 1));
	video_menu.push_back(new Item(Box(0.8, 0.8, 0.95, 0.9), "Apply", 2));
	
	audio_menu.push_back(new Label(Box(0.1, 0.1, 0.4, 0.2), "SFX Device"));
	audio_menu.push_back(new Label(Box(0.1, 0.2, 0.4, 0.3), "Music Device"));
	audio_menu.push_back(new Label(Box(0.1, 0.3, 0.4, 0.4), "OpenAL Context"));
	audio_menu.push_back(new StringChoice(Box(0.5, 0.1, 0.8, 0.2), audio_devs.begin(), audio_devs.end(), &sound_dev));
	audio_menu.push_back(new StringChoice(Box(0.5, 0.2, 0.8, 0.3), audio_devs.begin(), audio_devs.end(), &music_dev));
	audio_menu.push_back(new StringChoice(Box(0.5, 0.3, 0.8, 0.4), openal_context.begin(), openal_context.end(), &openal));
	audio_menu.push_back(new Item(Box(0.6, 0.8, 0.75, 0.9), "Cancel", 1));
	audio_menu.push_back(new Item(Box(0.8, 0.8, 0.95, 0.9), "Apply", 2));

	controls_menu.push_back(new Choice(Box(0.1, 0.1, 0.4, 0.2), &use_joystick, "Joystick on", "Joystick off"));
	controls_menu.push_back(new Choice(Box(0.5, 0.1, 0.8, 0.2), &joystick_debug, "Debug Joystick", "Normal Joy"));
	controls_menu.push_back(new Label(Box(0.1, 0.2, 0.4, 0.3), "Joystick cfg"));
	controls_menu.push_back(new StringBox(Box(0.5, 0.2, 0.8, 0.3), &joystick));
	controls_menu.push_back(new Item(Box(0.6, 0.8, 0.75, 0.9), "Cancel", 1));
	controls_menu.push_back(new Item(Box(0.8, 0.8, 0.95, 0.9), "Apply", 2));

	gfx::initializers::font_init->init();	
	menu_begin(gx, es, &sm);
}

Menu::~Menu()
{
	using namespace misc;

	menu_end();
	gfx::initializers::font_init->exit();	

	for_each(seq(back), delete_it);

	for_each(seq(main_menu), delete_it);
	for_each(seq(single_menu), delete_it);
	for_each(seq(multi_menu), delete_it);
	for_each(seq(options_menu), delete_it);
	for_each(seq(host_menu), delete_it);
	for_each(seq(join_menu), delete_it);
	for_each(seq(video_menu), delete_it);
	for_each(seq(audio_menu), delete_it);
	for_each(seq(controls_menu), delete_it);
}

void Menu::add_resolution(const Resolution &r)
{
	stringstream ss;
	ss << r.w << 'x' << r.h;
	res_choices.push_back(make_pair(ss.str(), r));
}

bool Menu::main()
{
	bool start_game = false;
	while(true) {
		MenuE m(gx, -1, -1, main_menu.begin(), main_menu.end() - 1, *back[0]);

		switch (m.run()) {
		case 1:	start_game = singleplayer();
			break;

		case 2:	//start_game = multiplayer();   
			break;

		case 3:	options();

		case 4 : // Movie/Intro
			break;
		
		case 0 : // Exit game.
			return false;
			
		}
		if(start_game) {
			return true;
		}
	}
}

bool Menu::singleplayer()
{
	while (true) {
		MenuE m(gx, -1, 0, single_menu.begin(), single_menu.end() - 1, *back[1]);
		switch (m.run()) {
		case 2 :// Start Game
			return true;
		
		case 0 :// Back
			return false;		
		}
	}		
}

bool Menu::multiplayer()
{
	while(true) {
		MenuE m(gx, -1, 0, multi_menu.begin(), multi_menu.end() - 1, *back[2]);

		switch (m.run()) {
		case 1:	if(host_multiplayer()) {
				return true;
			}
			break;

		case 2: if(join_multiplayer()) {
				return true;
			}
			break;

		case 0:	return false;
		}
	}
}

bool Menu::host_multiplayer() 
{
	while(true) {
		MenuE m(gx, -1, 0, host_menu.begin(), host_menu.end() - 1, *back[2]);

		switch(m.run()) {
		case 1 : // Change Player Name
			break;
		
		case 2 : // Name Of Server To Launch
			break;
		
		case 3 : // Choose Ship
			break;
		
		case 4 : // Choose Team
			break;
		
		case 5 : // Launch Multiplayer Game
			return true;
		
		case 0 : // Back
			return false;
		
		}
	}
}

bool Menu::join_multiplayer()
{
	while(true) {
		MenuE m(gx, -1, 0, join_menu.begin(), join_menu.end() - 1, *back[2]);

		switch(m.run()) {
		case 1: // Change Player Name
			break;
		
		case 2: // Name Of Server To Join
			break;
		
		case 3: // Choose ship
			break;
		
		case 4: // Choose Team
			break;
		
		case 5: // Join Multiplayer Game
			return true;
		
		case 0: // Back
			return false;		
		}
	}
}

void Menu::options()
{
	while (true) {
		MenuE m(gx, -1, 0, options_menu.begin(), options_menu.end() - 1, *back[3]);

		switch (m.run()){
		case 1: video_options();
			break;

		case 2: audio_options();
			break;

		case 3: control_options();
			break;

		case 0: return;
		}
	}
}

void Menu::video_options() 
{
	gfx_sets = gfx::Settings::read(true);
	while(true) {
		MenuE m(gx, -1, 1, video_menu.begin(), video_menu.end() - 1, *back[3]);

		switch(m.run()) {
		case 1: // Cancel
			return;		

		case 2: { // Apply
			derr << "Video settings written\n";
			gfx::Settings::write(gfx_sets);
			res::res_out_stream os(res::Config, "hw_gfx");
			boolalpha(os);
			os << "width      = " << resolution.w << "\r\n";
			os << "height     = " << resolution.h << "\r\n";
			os << "fullscreen = " << fullscreen << "\r\n";
			return;
			}
		}
	}
}

void Menu::audio_options() 
{
	while(true) {
		MenuE m(gx, -1, 1, audio_menu.begin(), audio_menu.end() - 1, *back[3]);

		switch(m.run()) {
		case 1: // Cancel
			return;
		
		case 2: { // Apply
			derr << "Audio settings written\n";
			res::res_out_stream os(res::Config, "hw_snd_system");
			os << "# dsound | openal | simple | dummy\r\n\r\n";
			os << "sound = " << sound_dev << "\r\n";
			os << "music = " << music_dev << "\r\n\r\n";
			os << "# DirectSound3D | DirectSound | Wave\r\n\r\n";
			os << "openal_context = " << openal << "\r\n";	
			return;
			}
		
		}
	}
}

void Menu::control_options() {
	while (true) {
		MenuE m(gx, -1, 1, controls_menu.begin(), controls_menu.end() - 1, *back[3]);

		switch(m.run()) {
		case 1: // Cancel
			return;
		
		case 2: { // Apply
			derr << "Control settings written\n";
			res::res_out_stream os(res::Config, "hw_input");
			os << "use_joystick = " << use_joystick << "\r\n";
			os << "joystick = " << joystick << "\r\n\r\n";
			os << "joystick_debug = " << joystick_debug << "\r\n";
			}
		}
	}
}

}

int test_main()
{
	bool run_game = false;
	do {
		{
		derr << "Entering menu...\n";
		reaper::Menu menu;
		run_game = menu.main();
		}
		if(run_game) {
			debug::debug_disable("world");

			hw::gfx::Gfx gx;

			auto_ptr<game::Game> go(new game::Game(gx));

			go->game_init();
			go->game_start();

			bool quit = false;

			while (!quit && !exit_now()) {
				quit = go->loop_once();
			}
			go->shutdown();

			derr << "Successful exit. Average FPS = "
			     << 1000000.0 / go->timers.frame.get_avg() << '\n';
		}
	} while(run_game);
	
	return 0;
}
