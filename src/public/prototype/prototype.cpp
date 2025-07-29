#include "hw/compat.h"
#include "hw/gl.h"
#include "hw/gfx.h"
#include "misc/test_main.h"
#include "game/menus.h"
#include "game/helpers.h"
#include "res/resource.h"
#include "res/res.h"
#include "res/config.h"
#include "game/state.h"
#include "hw/stacktrace.h"
#include <memory>

using namespace reaper;
using namespace object;

void single_player_game(bool load)
{
	hw::gfx::Gfx gx;

	if (load) {
		game::state::restore("last");
	}
	std::unique_ptr<game::Game> go = std::make_unique<game::Game>(gx, args);

	go->game_init();
	if (go->game_start()) {
		while (!go->loop_once() && !exit_now())
			;
	}
	go->shutdown();

	derr << "Successful exit. Average FPS = "
	     << 1000000.0 / go->timers.frame.get_avg() << '\n';
}

int test_main()
{
	Args::iterator k = args.find("-s");
	hw::load_symbols_prog((k == args.end()) ? argv[0] : k->second);

	hw::catch_segfault();

	Args::iterator i = args.find("-d");
	if (i != args.end())
		res::add_datapath(i->second);

	Args::iterator j = args.find("-p");
	if (j != args.end())
		res::add_pluginpath(j->second);

	Args::iterator ri = args.find("-r");
	if (ri != args.end()) {
		res::add_datapath(ri->second + "/data/");
		res::add_pluginpath(ri->second + "/plugins/");
	}
	if (args.count("-h") || args.count("-help") || args.count("--help")) {
		std::cout
		     << "Usage: " << argv[0] << " [OPTIONS]\n\n"
		     << "   -h       help\n"
		     << "   -f       skip menu and go directly to game\n"
		     << "   -g       print all debug messages on stdout\n"
		     << "   -r <dir> use alternate reaper-root directory\n"
		     << "   -d <dir> use alternate data directory\n"
		     << "   -p <dir> use alternate plugin directory\n"
		     << "   module:variable=value    set a config-variable\n"
		     << "\n\n"
		     << "Homepage:    http://www.dtek.chalmers.se/groups/reaper\n"
		     << "Mailinglist: reaper3d-devel@lists.sourceforge.net\n"
		     << "\n";
		return 0;
	}
	
	if (!res::sanity_check()) {
		derr << "\n\nInstallation error!\n\n"
		     << "You need to unpack the data package and\n"
		     << "the binary package (and/or a source package)\n"
		     << "to the same directory.\n\n";
		return 1;
	}

	int r = 0;
	bool run_game = true, quit = false;
	do {
		res::mgr().flush();
		if (!args.count("-f"))
			run_game = game::run_menu();
		else
			quit = true;
		if (run_game) {
			single_player_game(args.count("-l") != 0);
		}
	} while (!quit && run_game);
	res::mgr().flush();
	res::mgr().clear();
	return r;
}
 
