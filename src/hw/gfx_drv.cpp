

#include "compat.h"

#include <string>
#include <iostream>
#include <algorithm>
#include <functional>
#include <cctype>

#include "hw/debug.h"

#include "hw/gfx.h"
#include "hw/gfx_types.h"
#include "hw/gfx_driver.h"
#include "hw/exceptions.h"
#include "hw/interfaces.h"
#include "hw/gfxplugin.h"
#include "hw/gl.h"
#include "hw/gl_info.h"
#include "hw/osinfo.h"

#include "res/config.h"

#include "misc/sequence.h"
#undef None
#include "misc/parse.h"

#ifdef MONOLITHIC
extern "C" reaper::hw::lowlevel::Gfx_driver* create_gfx_x11(reaper::hw::ifs::Gfx* m);
extern "C" reaper::hw::lowlevel::Gfx_driver* create_gfx_glfw(reaper::hw::ifs::Gfx* m);
#endif

namespace reaper
{
namespace hw
{
namespace gfx
{
using std::string;

namespace { debug::DebugOutput derr("hw::gfx"); }

std::ostream& operator<<(std::ostream& os , const VideoMode& m)
{
	os << m.width << 'x' << m.height << ' ' << m.depth
	   << ' ' << m.frequency
	   << (m.fullscreen ? " fullscreen" : "")
	   << ' ' << m.id;
	return os;
}


class myless : public std::unary_function<const VideoMode&, bool> {
	const VideoMode& mode;
public:
	myless(const VideoMode& vm) : mode(vm) { }
	bool operator()(const VideoMode& vm) {
		derr << vm << ' ' << mode << '\n';
		return mode.width  > vm.width
		    && mode.height > vm.height
		    && mode.depth == vm.depth;
	}
};


void config_mode(VideoMode& vm, int w, int h, bool full)
{
	if (w != 0 && h != 0) {
		vm.width = w;
		vm.height = h;
		vm.fullscreen = full;
	}
}



/* Interface object for the plugin */

class Main
 : public ifs::Gfx
{
	debug::DebugOutput dr;
	std::set<VideoMode>& modes;
public:
	Main(std::set<VideoMode>& m) : dr("gfx_plugin", 5), modes(m)
	{ }
	debug::DebugOutput& derr() { return dr; }
	void add_mode(const VideoMode& mode)
	{
		modes.insert(mode);
	}
	std::string config(const std::string& lbl) {
		res::ConfigEnv env("hw_gfx");
		return env[lbl];
	}

};

struct Gfx_data
{
	GfxPlugin* gfx_plugin;
	std::set<VideoMode> modes;
	Main* main;

	Gfx_data()
	 : gfx_plugin(0), main(0)
	{ }
};

void Gfx::init()
{
	res::ConfigEnv cnf("hw_gfx");
	string pfx("gfx_");
	string sub(cnf["subsystem"]);

	if (sub.empty()) {
		if (os_name() == "win32")
			sub = "win32";
		else
			sub = "glfw";  // Default to GLFW instead of X11 for cross-platform support
	}
	data->main = new Main(data->modes);

#ifdef MONOLITHIC
	driver = create_gfx_glfw(data->main);  // Use GLFW driver by default instead of X11
#else
	data->gfx_plugin = new GfxPlugin();
	driver = data->gfx_plugin->create(pfx+sub, data->main);
#endif

	if (driver == 0)
		throw hw_error(string("Failed to create gfx subsystem."));

}

Gfx::Gfx(const VideoMode& vm)
 : data(new Gfx_data()), driver(0)
{
	init();
	if (!change_mode(vm))
		throw hw_fatal_error("Initialization failed");
}

Gfx::Gfx()
 : data(new Gfx_data()), driver(0)
{
	init();

	VideoMode vm(800, 600);

	res::ConfigEnv cnf("hw_gfx");
	vm.fullscreen = cnf["fullscreen"];
	int width = cnf["width"];
	int height = cnf["height"];
	if (width > 0 && height > 0) {
		vm.width = width;
		vm.height = height;
	}
	
	std::set<VideoMode>::iterator i = misc::find(misc::seq(data->modes), vm);
	if (i != data->modes.end())
		vm = *i;

	if (!change_mode(vm))
		throw hw_fatal_error("Initialization failed");

	if (! driver->internal_data()->is_accelerated || bool(cnf["noaccel"])) {
		vm = VideoMode(400, 300);
		config_mode(vm, cnf["noaccel_width"], cnf["noaccel_height"], cnf["noaccel_fullscreen"]);
		change_mode(vm);
	}
}

Gfx::~Gfx()
{
	driver->restore_mode();
	delete driver;
	delete data->gfx_plugin;
	delete data->main;
	delete data;
}


bool Gfx::change_mode(const VideoMode& mode)
{
	VideoMode vm = mode;
	if (mode.depth == 0) {
		vm.depth = driver->internal_data()->current.depth;
	}
	if (mode.fullscreen) {
		if (vm.id == -1) {
			mode_iter i = misc::find(misc::seq(data->modes), vm);
			if (i == data->modes.end())
				vm = mode; //throw hw_error("Could not find mode");
			else
				vm = *i;
		}
	}
	driver->restore_mode();

	derr << "Videomode: " << vm;
	if(alpha())
		derr << " alpha";
	if(stencil())
		derr << " stencil";
	derr << "\n";

	bool result = driver->setup_mode(vm);
	if (result)
		hw::gfx::reinit_opengl();

	return result;
}


void Gfx::update_screen()
{
	driver->update_screen();
}

const VideoMode& Gfx::current_mode() const
{
	return driver->internal_data()->current;
}

bool Gfx::is_accelerated() const
{
	return driver->internal_data()->is_accelerated;
}

bool Gfx::alpha() const
{
	return driver->internal_data()->alpha;
}

bool Gfx::stencil() const
{
	return driver->internal_data()->stencil;
}



}
}
}

