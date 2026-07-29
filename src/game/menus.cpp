

#include <iostream>
#include <deque>
#include <memory>

#include "hw/debug.h"
#include "hw/snd.h"
#include "hw/gl.h"

#include "misc/menu.h"
#include "misc/font.h"
#include "misc/sequence.h"
#include "misc/free.h"
#include "misc/stlhelper.h"

#include "gfx/settings.h"
#include "gfx/managers.h"
#include "gfx/abstracts.h"
#include "gfx/light.h"

#include "res/res.h"
#include "res/config.h"
#include "res/resource.h"

#include <string>
#include <sstream>
#include <cctype>

namespace reaper {
namespace {

using namespace menu;
using namespace std;

using res::resource;
using res::save;
using res::ConfigEnv;

//////////////////////////////////////////////////////////////////////////////
// Simple class for displaying a mesh, rotated with lighting
template<class V>
class MenuItemMeshView : public MenuItem<V>
{
public:
	typedef string (*TRANSLATE_FUNC)(const string &mesh);
private:
	gfx::MeshRef mr;
	gfx::TextureRef tr;

	float angle;
	hw::time::TimeSpan time;
	const string& mesh;
	TRANSLATE_FUNC translate;
	Matrix mtx;
	gfx::RenderInfo ri;

	gfx::light::Light light;
public:
	MenuItemMeshView(Box b, const string& msh, TRANSLATE_FUNC f);
	virtual int draw_inactive();
};

//////////////////////////////////////////////////////////////////////////////
string object_to_mesh(const string &obj)
{	
	return obj.substr(0,5);
}

inline pair<string, string> make_strp(const string &s1, const string &s2) {
	return make_pair(s1, s2);
}

reaper::debug::DebugOutput dout("menu",0);

//////////////////////////////////////////////////////////////////////////////

enum RetAction { Quit, Back, InMenu, Apply, Start, Invalid };

typedef MenuItem<RetAction> BaseItem;
typedef MenuItemLabel<RetAction> Label;
typedef MenuItemHeader<RetAction> Header;
typedef MenuItemRetVal<RetAction> Item;
typedef MenuItemStringBox<RetAction> StringBox;
typedef MenuItemTwoChoice<RetAction> Choice;
typedef MenuItemMeshView<RetAction> MeshView;

typedef deque<misc::SmartPtr<BaseItem> > MenuCont;
typedef Menuexec<MenuCont::iterator, RetAction> MenuE;

//////////////////////////////////////////////////////////////////////////////

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
typedef vector<pair<string, int> > IntCont;

typedef MenuItemMultiChoice<RetAction, ResolutionCont::iterator, Resolution> ResChoice;
typedef MenuItemMultiChoice<RetAction, StringCont::iterator, string> StringChoice;
typedef MenuItemMultiChoice<RetAction, IntCont::iterator, int> IntChoice;

//////////////////////////////////////////////////////////////////////////////

using Textures = deque<std::shared_ptr<Texture>>;

class Menu {
public:
	virtual RetAction run() = 0;
	virtual ~Menu() { }
};

typedef Menu* MenuPtr;
typedef MenuItemCallback<RetAction,MenuPtr,Menu> Callback;

class MenuImpl {
protected:
	MenuSystem& ms;
	Textures backs;
	MenuCont menu;

	MenuImpl(MenuSystem& m, Textures bs) : ms(m), backs(bs) { }
	virtual ~MenuImpl() { }
};

const Box box;
const Box title_box(0.1, 0.1, 0.9, 0.2);
const Box center_box(0.1, 0.3, 0.9, 0.85);
const Box left_box(0.2, 0.25, 0.5, 0.7);
const Box right_box(0.5, 0.25, 0.8, 0.7);
const Box lr_box(0.85, 0.85, 0.95, 0.9);   // Lower Right corner box
const Box lolr_box(0.7, 0.85, 0.8, 0.9);   // Left Of lower right corner box

class NotImplMenu : public Menu, private MenuImpl
{


public:
	NotImplMenu(MenuSystem& ms, Textures backs)
	 : MenuImpl(ms, backs)
	{
		menu.push_back(std::make_shared<Header>(Box(0.1, 0.45, 0.9, 0.55), "Not Implemented"));
		menu.push_back(std::make_shared<Item>(lr_box, "Back", Back));
	}
	RetAction run() {
		MenuE m(ms, Invalid, Back, menu.begin(), menu.end(), *backs[3]);
		m.run();
		return InMenu;
	}
};

struct Ships {
	StringCont ships;
	Ships() {
		const char* ship_data[] = {
			"Ship X", "shipXP",
			"Ship 2", "ship2P",
			"Ship 3", "ship3P",
			"Ship 4", "ship4P"  };
		for (int i = 0; i < 4; ++i) {
			pair<string,string> p;
			try {
				p = make_strp(ship_data[2*i], ship_data[2*i+1]);
				gfx::MeshRef()->load(object_to_mesh(p.second));
				ships.push_back(p);
			}
			catch (error_base& e) { dout << e.what() << '\n'; }
		}
	}
	const StringCont& get() { return ships; }
};

const StringCont& available_ships()
{
	static Ships ss;
	return ss.get();
}


struct Levels {
	StringCont levels;
	Levels() {
		const char* data[] = {
			"Level1", "test_level4",
			"Tiny", "tiny_level",
			"Big", "big_level" };
		for (int i = 0; i < 3; ++i) {
			pair<string,string> p;
			try {
				p = make_strp(data[2*i], data[2*i+1]);
				const auto env =
					res::resource_ptr<res::ConfigEnv>(
						"levels/" + p.second + ".rl");
				if (env->defined("Scenario"))
					levels.push_back(p);
			} catch (...) { }
		}
	}
	const StringCont& get() { return levels; }
};

const StringCont& available_levels()
{
	static Levels ls;
	return ls.get();
}




class SingleMenu : public Menu, private MenuImpl
{
	string player_name;
	StringCont levels;
	StringCont ships;
	string player_ship;
	string level;
public:
	SingleMenu(MenuSystem& ms, Textures backs)
	 : MenuImpl(ms, backs),
	   player_name("Rookie One"),
	   levels(available_levels()),
	   ships(available_ships()),
	   player_ship("<not found>"),
	   level("<not found>")
	{
		if (!ships.empty())
			player_ship = ships.front().second;
		if (!levels.empty())
			level = levels.front().second;
		misc::push_back(menu)
		 << std::make_shared<Header>(title_box, "Single Player")
		 << std::make_shared<Label>(box, "Player Name")
		 << std::make_shared<Label>(box, "Player Ship")
		 << std::make_shared<Label>(box, "Level")
		 << std::make_shared<StringBox>(box, &player_name)
		 << std::make_shared<StringChoice>(box, ships.begin(), ships.end(), &player_ship)
		 << std::make_shared<StringChoice>(box, levels.begin(), levels.end(), &level)
		 << std::make_shared<Item>(Box(0.35, 0.6, 0.65, 0.65), "Start", Start)
		 << std::make_shared<MeshView>(Box(0.35, 0.65, 0.65, 0.95), player_ship, object_to_mesh)
		 << std::make_shared<Item>(lr_box, "Back", Back);
		vertical_layout(left_box, menu.begin()+1, 3);
		vertical_layout(right_box, menu.begin()+4, 3);
	}
	RetAction run() {
		MenuE m(ms, Invalid, Back, menu.begin(), menu.end(), *backs[1]);
		RetAction r = m.run();
		if (r == Start) {
			string pfx = ConfigEnv("game")["active"];
			res::ConfigEnv& env = res::resource<res::ConfigEnv>("game");
			env[pfx+"_player_ships"] = player_ship;
			env[pfx+"_level"] = level;
			
			return Start;
		} else {
			return InMenu;
		}
	}
};

class TwoPlayerMenu : public Menu, private MenuImpl
{
	StringCont ships;
	string player_ship_1, player_ship_2;
public:
	TwoPlayerMenu(MenuSystem& ms, Textures backs)
	 : MenuImpl(ms, backs),
	   ships(available_ships()),
	   player_ship_1("<not found>"),
	   player_ship_2("<not found>")
	{
		if (!ships.empty()) {
			player_ship_1 = ships.front().second;
			player_ship_2 = ships.back().second;
		}

		misc::push_back(menu)
		 << std::make_shared<Header>(title_box, "Two Player Game")
		 << std::make_shared<Label>(Box(0.25, 0.25, 0.45, 0.30), "Player 1 Ship")
		 << std::make_shared<StringChoice>(Box(0.55, 0.25, 0.8, 0.30), ships.begin(), ships.end(), &player_ship_1)
		 << std::make_shared<Label>(Box(0.25, 0.35, 0.45, 0.40), "Player 2 Ship")
		 << std::make_shared<StringChoice>(Box(0.55, 0.35, 0.8, 0.40), ships.begin(), ships.end(), &player_ship_2)
		 << std::make_shared<Item>(Box(0.35, 0.45, 0.65, 0.50), "Start", Start);

		auto i1 = std::make_shared<MeshView>(
			Box(0.15, 0.55, 0.45, 0.85), player_ship_1, object_to_mesh);
		menu.push_back(i1);
		auto i2 = std::make_shared<MeshView>(
			Box(0.55, 0.55, 0.85, 0.85), player_ship_2, object_to_mesh);
		menu.push_back(i2);

		menu.push_back(std::make_shared<Item>(lr_box, "Back", Back));
	}
	RetAction run() {
		MenuE m(ms, Invalid, Back, menu.begin(), menu.end(), *backs[1]);
		RetAction r = m.run();
		if (r == Start) {
			string pfx = ConfigEnv("game")["active"];
			res::dumper(res::resource<res::ConfigEnv>("game"))
				(pfx+"_player_ships", player_ship_1 + " " + player_ship_2)
				(pfx+"_split_screen", true);
			return Start;
		} else {
			return InMenu;
		}
	}
};

class NetworkHostMenu : public Menu, private MenuImpl
{
//	StringCont ships;
	string player_name;
	StringCont levels;
	string server;
//	string player_ship;
	string level;
public:
	NetworkHostMenu(MenuSystem& ms, Textures backs)
	 : MenuImpl(ms, backs),
	   player_name("Rookie One"),
	   levels(available_levels()),
	   server("localhost"),
	   level("<not found>")
//	,  player_ship("shipXP")
	{
		if (!levels.empty())
			level = levels.front().second;

		misc::push_back(menu)
		 << std::make_shared<Header>(title_box, "Network Game")
		 << std::make_shared<Label>(box, "Player Name")
//		 << std::make_shared<Label>(box, "Player Ship")
		 << std::make_shared<Label>(box, "Level")
		 << std::make_shared<Label>(box, "Server:port")
		 << std::make_shared<StringBox>(box, &player_name)
//		 << std::make_shared<StringChoice>(box, ships.begin(), ships.end(), &player_ship)
		 << std::make_shared<StringChoice>(box, levels.begin(), levels.end(), &level)
		 << std::make_shared<StringBox>(box, &server)
		 << std::make_shared<Item>(Box(0.35, 0.65, 0.65, 0.70), "Start", Start);
		vertical_layout(left_box, menu.begin()+1, 3);
		vertical_layout(right_box, menu.begin()+4, 3);
		menu.push_back(std::make_shared<Item>(lr_box, "Back", Back));
	}
	RetAction run() {
		MenuE m(ms, Invalid, Back, menu.begin(), menu.end(), *backs[1]);
		RetAction r = m.run();
		if (r == Start) {
			string pfx = ConfigEnv("game")["active"];
			res::dumper(res::resource<res::ConfigEnv>("game"))
				(pfx+"_player_name", player_name)
				(pfx+"_level", level)
				(pfx+"_scenario", std::string("none"))
				(pfx+"_networked", true)
				(pfx+"_fixed_dt", true)
				(pfx+"_server", true)
				(pfx+"_servername", std::string("localhost"));
			return Start;
		} else {
			return InMenu;
		}
	}
};

class NetworkClientMenu : public Menu, private MenuImpl
{
//	StringCont ships;
	string player_name;
	StringCont levels;
	string server;
//	string player_ship;
	string level;
public:
	NetworkClientMenu(MenuSystem& ms, Textures backs)
	 : MenuImpl(ms, backs),
	   player_name("Rookie One"),
	   levels(available_levels()),
	   server("localhost"),
	   level("<not found>")
//	,  player_ship("shipXP")
	{
		if (!levels.empty())
			level = levels.front().second;

		misc::push_back(menu)
		 << std::make_shared<Header>(title_box, "Network Game")
		 << std::make_shared<Label>(box, "Player Name")
//		 << std::make_shared<Label>(box, "Player Ship")
		 << std::make_shared<Label>(box, "Level")
		 << std::make_shared<Label>(box, "Server")
		 << std::make_shared<StringBox>(box, &player_name)
//		 << std::make_shared<StringChoice>(box, ships.begin(), ships.end(), &player_ship)
		 << std::make_shared<StringChoice>(box, levels.begin(), levels.end(), &level)
		 << std::make_shared<StringBox>(box, &server)
		 << std::make_shared<Item>(Box(0.35, 0.65, 0.65, 0.70), "Start", Start);
		vertical_layout(left_box, menu.begin()+1, 3);
		vertical_layout(right_box, menu.begin()+4, 3);
		menu.push_back(std::make_shared<Item>(lr_box, "Back", Back));
	}
	RetAction run() {
		MenuE m(ms, Invalid, Back, menu.begin(), menu.end(), *backs[1]);
		RetAction r = m.run();
		if (r == Start) {
			string pfx = ConfigEnv("game")["active"];
			res::dumper(res::resource<res::ConfigEnv>("game"))
				(pfx+"_player_name", player_name)
				(pfx+"_level", level)
				(pfx+"_scenario", std::string("none"))
				(pfx+"_networked", true)
				(pfx+"_fixed_dt", true)
				(pfx+"_server", false)
				(pfx+"_servername", server);
			return Start;
		} else {
			return InMenu;
		}
	}
};

class VideoOptMenu : public Menu, private MenuImpl
{
	gfx::Settings gfx_sets;
	bool fullscreen;
	Resolution resolution;
	ResolutionCont res_choices;
	IntCont shadow_type;
	IntCont texture_detail;
	IntCont texture_level;

	deque<Resolution> resolutions;

	void add_resolution(const Resolution& r) {
		stringstream ss;
		ss << r.w << 'x' << r.h;
		res_choices.push_back(make_pair(ss.str(), r));
	}
public:
	VideoOptMenu(MenuSystem& ms, Textures backs)
	 : MenuImpl(ms, backs),
	   gfx_sets(gfx::Settings::read(true)),
	   resolution(640, 480)
	{
		misc::push_back(resolutions)
		 << Resolution(320, 200)
		 << Resolution(400, 300)
		 << Resolution(640, 480)
		 << Resolution(800, 600)
		 << Resolution(1024, 768)
		 << Resolution(1280, 960)
		 << Resolution(1280, 1024)
		 << Resolution(1600, 1200);

		misc::push_back(shadow_type)
		 << make_pair(string("No shadows"), 0)
		 << make_pair(string("Simple player"), 1)
		 << make_pair(string("Simple ships"), 2);
		if(ms.gfx().alpha()) {
	       		misc::push_back(shadow_type)
	       		 << make_pair(string("Nice player Simple ships"), 3)
	       		 << make_pair(string("Nice ships"), 4)
	       		 << make_pair(string("Nice all"), 5);
	        }
		misc::push_back(texture_detail)
		 << make_pair(string("Maximum texdetail"), 1)
		 << make_pair(string("High texdetail"), 2)
		 << make_pair(string("Medium texdetail"), 4)
		 << make_pair(string("Low texdetail"), 8)
		 << make_pair(string("Minimum texdetail"), 16);

		misc::push_back(texture_level)
		 << make_pair(string("Texture on"), 2)
		 << make_pair(string("Object-textures off"), 1)
		 << make_pair(string("Texturing off"), 0);

		// Setup resolution stuff
		for(deque<Resolution>::const_iterator i = resolutions.begin(); i != resolutions.end(); ++i) {
			add_resolution(*i);
		}
		ConfigEnv hw_gfx("hw_gfx");
		resolution.w = res::withdefault(hw_gfx, "width", 640);
		resolution.h = res::withdefault(hw_gfx, "height", 480);
		fullscreen   = res::withdefault(hw_gfx, "fullscreen", true);

		// If a special resolution is entered manually into hw_gfx settings file,
		// add it to the list of possible choices
		ResolutionCont::iterator i;
		for(i = res_choices.begin(); i != res_choices.end(); ++i) {
			if(i->second == resolution) {
				break;
			}
		}
		if(i == res_choices.end()) {
			add_resolution(resolution);
		}
		misc::push_back(menu)
		 << std::make_shared<IntChoice>(box, texture_level.begin(), texture_level.end(), &gfx_sets.texture_level)
		 << std::make_shared<Choice>(box, &gfx_sets.use_lighting, "Lighting on", "Lighting off")
		 << std::make_shared<Choice>(box, &gfx_sets.use_fog, "Fog on", "Fog off")
		 << std::make_shared<Choice>(box, &gfx_sets.draw_shadows, "Shadows on", "Shadows off")
		 << std::make_shared<Choice>(box, &gfx_sets.draw_lights, "Lights on", "Lights off")
		 << std::make_shared<ResChoice>(box, res_choices.begin(), res_choices.end(), &resolution)
		 << std::make_shared<Choice>(box, &fullscreen, "Fullscreen", "Windowed")
		 << std::make_shared<Choice>(box, &gfx_sets.draw_terrain, "Terrain on", "Terrain off")
		 << std::make_shared<Choice>(box, &gfx_sets.draw_effects, "Effects on", "Effects off")
		 << std::make_shared<Choice>(box, &gfx_sets.draw_sky, "Sky on", "Sky off")
		 << std::make_shared<Choice>(box, &gfx_sets.draw_water, "Water on", "Water off")
		 << std::make_shared<Choice>(box, &gfx_sets.test_for_errors, "Trap GL errors", "No debug")
		 << std::make_shared<IntChoice>(box, texture_detail.begin(), texture_detail.end(), &gfx_sets.texture_scaling)
		 << std::make_shared<Choice>(box, &gfx_sets.use_arb_texture_compression, "TexCompr On", "TexCompr Off")
		 << std::make_shared<IntChoice>(box, shadow_type.begin(), shadow_type.end(), &gfx_sets.shadow_type);
		vertical_layout(center_box, menu.begin(), menu.size());

		misc::push_back(menu)
		 << std::make_shared<Header>(title_box, "Video Options")
		 << std::make_shared<Item>(lolr_box, "Apply", Apply)
		 << std::make_shared<Item>(lr_box, "Cancel", Back);
	}
	RetAction run() {
		MenuE m(ms, Invalid, Back, menu.begin(), menu.end(), *backs[3]);
		while (true) {
			switch (m.run()) {
			case Apply: {
				dout << "Video settings written\n";
				gfx::Settings::write(gfx_sets);
				ConfigEnv& env = res::resource<ConfigEnv>("hw_gfx");
				env["width"] = resolution.w;
				env["height"] = resolution.h;
				env["fullscreen"] = fullscreen;
				res::save<ConfigEnv>("hw_gfx");
				}
				return InMenu;
			case Back: return InMenu;
			default: break;
			}
		}
	}
};

class AudioOptMenu : public Menu, private MenuImpl
{
	string sound_dev;
	string openal;
	StringCont audio_devs_game;
	StringCont audio_devs_stream;
	string music_dev;
	StringCont openal_context;

public:
	AudioOptMenu(MenuSystem& ms, Textures backs)
	 : MenuImpl(ms, backs)
	{
		ConfigEnv hw_snd_system("hw_snd_system");
		sound_dev = res::withdefault(hw_snd_system, "sound", "openal");
		music_dev = res::withdefault(hw_snd_system, "music", "simple");
		openal    = res::withdefault(hw_snd_system, "openal_context", "");
		misc::push_back(audio_devs_game)
		 << make_strp("Disabled", "dummy")
		 << make_strp("Simple", "simple")
		 << make_strp("OpenAL", "openal")
		 << make_strp("DSound", "dsound");

		misc::push_back(audio_devs_stream)
		 << make_strp("Disabled", "dummy")
		 << make_strp("OpenAL", "openal");

		misc::push_back(openal_context)
		 << make_strp("System Default", "");

		misc::push_back(menu)
		 << std::make_shared<Label>(box, "SFX Device")
		 << std::make_shared<Label>(box, "Music Device")
		 << std::make_shared<Label>(box, "OpenAL Device")
		 << std::make_shared<StringChoice>(box, audio_devs_game.begin(), audio_devs_game.end(), &sound_dev)
		 << std::make_shared<StringChoice>(box, audio_devs_stream.begin(), audio_devs_stream.end(), &music_dev)
		 << std::make_shared<StringChoice>(box, openal_context.begin(), openal_context.end(), &openal)
		 << std::make_shared<Header>(title_box, "Audio Options")
		 << std::make_shared<Item>(lolr_box, "Apply", Apply)
		 << std::make_shared<Item>(lr_box, "Cancel", Back);
		vertical_layout(Box(0.1, 0.3, 0.4, 0.8), menu.begin(), 3);
		vertical_layout(Box(0.4, 0.3, 0.9, 0.8), menu.begin()+3, 3);
	}
	RetAction run() {
		MenuE m(ms, Invalid, Back, menu.begin(), menu.end(), *backs[3]);
		while (true) {
			switch(m.run()) {
			case Apply: {
				ConfigEnv& env = res::resource<ConfigEnv>("hw_snd_system");
				env["sound"] = sound_dev;
				env["music"] = music_dev;
				env["openal_context"] = openal;
				res::save<ConfigEnv>("hw_snd_system");
				dout << "Audio settings saved\n";
				}
				return InMenu;
			case Back: return InMenu;
			default: break;	
			}
		}
	}
};

class ControlOptMenu : public Menu, private MenuImpl
{
	bool use_joystick;
	bool joystick_debug;	

	string joystick;


public:
	ControlOptMenu(MenuSystem& ms, Textures backs)
	 : MenuImpl(ms, backs)
	{
		ConfigEnv hw_input("hw_input");
		use_joystick = res::withdefault(hw_input, "use_joystick", false);
		joystick     = res::withdefault(hw_input, "joystick", "hw_joystick_standard");
		joystick_debug = res::withdefault(hw_input, "joystick_debug", false);
		misc::push_back(menu)
		 << std::make_shared<Choice>(box, &use_joystick, "Joystick on", "Joystick off")
		 << std::make_shared<Choice>(box, &joystick_debug, "Debug Joystick", "Normal Joy")
		 << std::make_shared<Label>(box, "Joystick cfg")
		 << std::make_shared<StringBox>(box, &joystick)
		 << std::make_shared<Header>(title_box, "Control Options")
		 << std::make_shared<Item>(lolr_box, "Apply", Apply)
		 << std::make_shared<Item>(lr_box, "Cancel", Back);
		vertical_layout(center_box, menu.begin(), menu.size()-3);
	}
	RetAction run() {
		MenuE m(ms, Invalid, Back, menu.begin(), menu.end(), *backs[3]);
		while (true) {
			switch(m.run()) {
			case Apply: {
				ConfigEnv& env = res::resource<ConfigEnv>("hw_input");
				env["use_joystick"] = use_joystick;
				env["joystick"] = joystick;
				env["joystick_debug"] = joystick_debug;
				res::save<ConfigEnv>("hw_input");
				dout << "Control settings saved\n";
				}
				return InMenu;
			case Back: return InMenu;
			default: break;
			}
		}
	}
};

class OptionsMenu : public Menu, private MenuImpl
{
	std::unique_ptr<Menu> video;
	std::unique_ptr<Menu> audio;
	std::unique_ptr<Menu> control;
public:
	OptionsMenu(MenuSystem& ms, Textures backs)
	 : MenuImpl(ms, backs),
	   video(std::make_unique<VideoOptMenu>(ms, backs)),
	   audio(std::make_unique<AudioOptMenu>(ms, backs)),
	   control(std::make_unique<ControlOptMenu>(ms, backs))
	{
		misc::push_back(menu)
		 << std::make_shared<Header>(title_box, "Options")
		 << std::make_shared<Callback>(Box(), "Video", video.get(), &Menu::run)
		 << std::make_shared<Callback>(Box(), "Audio", audio.get(), &Menu::run)
		 << std::make_shared<Callback>(Box(), "Controls", control.get(), &Menu::run)
		 << std::make_shared<Item>(lr_box, "Back", Back);
		vertical_layout(center_box, menu.begin()+1, 3);
	}

	RetAction run() {
		MenuE m(ms, Invalid, Back, menu.begin(), menu.end(), *backs[3]);
		while (m.run() != Back)
			;
		return InMenu;
	}
};

class MainMenu : public Menu, private MenuImpl
{
	std::unique_ptr<Menu> not_impl;
	std::unique_ptr<Menu> single;
	std::unique_ptr<Menu> two_player;
	std::unique_ptr<Menu> options;
	std::unique_ptr<Menu> net_host;
	std::unique_ptr<Menu> net_client;
public:
	MainMenu(MenuSystem& ms, Textures backs) 
	 : MenuImpl(ms, backs),
	   not_impl(std::make_unique<NotImplMenu>(ms, backs)),
	   single(std::make_unique<SingleMenu>(ms, backs)),
	   two_player(std::make_unique<TwoPlayerMenu>(ms, backs)),
	   options(std::make_unique<OptionsMenu>(ms, backs)),
	   net_host(std::make_unique<NetworkHostMenu>(ms, backs)),
	   net_client(std::make_unique<NetworkClientMenu>(ms, backs))
	{
		misc::push_back(menu)
			<< std::make_shared<Callback>(box, "Single player", single.get(), &Menu::run)
			<< std::make_shared<Callback>(box, "Two player", two_player.get(), &Menu::run)
//			<< std::make_shared<Callback>(box, "Network Host", net_host, &Menu::run)
//			<< std::make_shared<Callback>(box, "Network Client", net_client, &Menu::run)
			<< std::make_shared<Callback>(box, "Options", options.get(), &Menu::run)
			<< std::make_shared<Header>(title_box, "Main Menu")
			<< std::make_shared<Item>(lr_box, "Quit", Quit);
		vertical_layout(center_box, menu.begin(), menu.size() - 2);
	}

	RetAction run() {
		MenuE m(ms, Invalid, Quit, menu.begin(), menu.end(), *backs[0]);
		while (true) {
			switch (m.run()) {
				case Quit:  return Quit;
				case Start: return Start;
				default:    break;
			}
		}
	}
};

class ReaperMenu  {

	MenuSystem ms;

	// Blah...
	Textures backs;

	std::unique_ptr<Menu> main;
public:
	ReaperMenu();

	bool run_main();
};

ReaperMenu::ReaperMenu()
{
	gfx::init_gl_settings(gfx::Settings::current);
	misc::push_back(backs)
		<< std::make_shared<Texture>("menu_bkg_1")
		<< std::make_shared<Texture>("menu_bkg_2")
		<< std::make_shared<Texture>("menu_bkg_3")
		<< std::make_shared<Texture>("menu_bkg_4");
	main = std::make_unique<MainMenu>(ms, backs);
}

bool ReaperMenu::run_main()
{
	return main->run() == Start;
}



template<class V>
MenuItemMeshView<V>::MenuItemMeshView(Box b, const string& msh, TRANSLATE_FUNC f) :
	menu::MenuItem<V>(b, msh, true, false),
	angle(0), 
	time(hw::time::get_time()),
	mesh(msh),
	translate(f),
	mtx(Matrix::id()),
	ri(translate(mesh), mtx, false),
	light(norm(Vector(.05, 1, -.1)), Color(1,1,1))
{}

template<class V>
int MenuItemMeshView<V>::draw_inactive() 
{
	hw::time::TimeSpan time2(hw::time::get_time());
	float dt = (time2 - time).approx().to_s();
	time = time2;
	angle += dt * 90;

	if(ri.mesh != misc::Unique(translate(mesh))) {
		ri = gfx::RenderInfo(translate(mesh), mtx, false);
	}

	float inv_r = 1.0f / ri.radius;

	glClear(GL_DEPTH_BUFFER_BIT);

	const menu::Box &b = this->get_box();
	
	glPushMatrix();		
	glTranslatef(b.x1, 1 - b.y1, 0);
	glScalef((b.x2 - b.x1) / 2, (b.y2 - b.y1) / 2, 1);		
	glTranslatef(1, -1, 0);
	glScalef(inv_r, inv_r, inv_r);	
	
	StateKeeper s5(GL_NORMALIZE, true);
	light.activate(0);
	glRotatef(30, 1, 0, 0);
	glRotatef(angle, 0, 1, 0);

	{
	StateKeeper s1(GL_LIGHTING, true);	
	StateKeeper s2(GL_LIGHT0, true);
	StateKeeper s3(GL_DEPTH_TEST, true);
	StateKeeper s4(GL_CULL_FACE, true);		

	tr->use(ri.texture);
	mr->render(ri, 0);
	}

	glPopMatrix();

	tr->use("");
	
	return 1;
}



} // end anonoymous namespace

namespace game {
bool run_menu() {
	gfx::TextureRef::create();
	gfx::MeshRef::create();
	bool r;
	{
		reaper::ReaperMenu menu;
		r = menu.run_main();
	}
	gfx::MeshRef::destroy();
	gfx::TextureRef::destroy();
	return r;
}
}

} // end reaper namespace
