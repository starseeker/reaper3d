/*
 * Reaper3D Game Engine - Main Application Entry Point
 * 
 * This file contains the main startup sequence for the Reaper3D game engine.
 * The startup flow has been re-wired to make GLFW initialization and 
 * main game loop startup explicit and clear.
 * 
 * Startup Sequence:
 * 1. main() -> test_main() handles command line args and configuration
 * 2. single_player_game() handles the core startup sequence:
 *    - Graphics system initialization (GLFW, window, OpenGL context)
 *    - Game object creation and subsystem initialization
 *    - Explicit main game loop startup
 * 
 * The old plugin architecture has been removed in favor of direct GLFW integration.
 */

#include "hw/compat.h"
#include "hw/gl.h"
#include "hw/gfx.h"
#include "hw/debug.h"
#include "hw/exceptions.h"
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
	debug::DebugOutput local_derr("prototype_startup");
	
	// Step 1: Initialize graphics system (including GLFW, window, and OpenGL context)
	// This is now the first explicit step to set up display and input handling
	local_derr << "Initializing graphics system with GLFW windowing...\n";
	hw::gfx::Gfx gx;
	local_derr << "Graphics system and window created successfully\n";

	// Step 2: Restore saved game state if requested
	if (load) {
		local_derr << "Restoring saved game state...\n";
		game::state::restore("last");
	}

	// Step 3: Create the main game object
	// This sets up all game subsystems (world, physics, sound, etc.)
	local_derr << "Creating game object and initializing subsystems...\n";
	std::unique_ptr<game::Game> go = std::make_unique<game::Game>(gx, args);

	// Step 4: Initialize game subsystems
	// This loads the world, sets up physics, renderer, sound, scenario, etc.
	local_derr << "Initializing game subsystems (world, physics, sound, scenario)...\n";
	go->game_init();

	// Step 5: Start the game and prepare for main loop
	// This finalizes startup, creates players, handles networking setup
	local_derr << "Starting game and preparing for main loop...\n";
	if (go->game_start()) {
		// Step 6: Run the main game loop explicitly
		// Process events, update simulation, render frames until exit
		local_derr << "Entering main game loop - game logic now running\n";
		while (!go->loop_once() && !exit_now()) {
			// Main game loop runs here - each iteration:
			// - Processes input events from GLFW
			// - Updates physics simulation
			// - Renders the frame using OpenGL
			// - Handles networking updates
		}
	}

	// Step 7: Clean shutdown of game systems
	local_derr << "Game loop ended, shutting down game systems...\n";
	go->shutdown();

	// Graphics system (including GLFW) will be cleaned up automatically
	// when the Gfx object goes out of scope

	local_derr << "Successful exit. Average FPS = "
	     << 1000000.0 / go->timers.frame.get_avg() << '\n';
}

int test_main()
{
	// Initialize crash handling and debugging
	Args::iterator k = args.find("-s");
	hw::load_symbols_prog((k == args.end()) ? argv[0] : k->second);
	hw::catch_segfault();

	// Set up resource paths
	Args::iterator i = args.find("-d");
	if (i != args.end())
		res::add_datapath(i->second);

	Args::iterator j = args.find("-p");
	// -p option removed (plugin paths no longer supported)

	Args::iterator ri = args.find("-r");
	if (ri != args.end()) {
		res::add_datapath(ri->second + "/data/");
	}
	
	// Handle help request
	if (args.count("-h") || args.count("-help") || args.count("--help")) {
		std::cout
		     << "Usage: " << argv[0] << " [OPTIONS]\n\n"
		     << "   -h       help\n"
		     << "   -f       skip menu and go directly to game\n"
		     << "   -g       print all debug messages on stdout\n"
		     << "   -r <dir> use alternate reaper-root directory\n"
		     << "   -d <dir> use alternate data directory\n"
		     << "   module:variable=value    set a config-variable\n"
		     << "\n\n"
		     << "Homepage:    http://www.dtek.chalmers.se/groups/reaper\n"
		     << "Mailinglist: reaper3d-devel@lists.sourceforge.net\n"
		     << "\n";
		return 0;
	}
	
	// Verify installation and data files
	if (!res::sanity_check()) {
		derr << "\n\nInstallation error!\n\n"
		     << "You need to unpack the data package and\n"
		     << "the binary package (and/or a source package)\n"
		     << "to the same directory.\n\n";
		return 1;
	}

	// Main application loop (menu system + game)
	int r = 0;
	bool run_game = true, quit = false;
	do {
		res::mgr().flush();
		
		// Show menu unless -f flag is used to skip directly to game
		if (!args.count("-f"))
			run_game = game::run_menu();
		else
			quit = true;
			
		// Start the actual game with the new explicit startup sequence
		if (run_game) {
			single_player_game(args.count("-l") != 0);
		}
	} while (!quit && run_game);
	
	// Clean up resources
	res::mgr().flush();
	res::mgr().clear();
	return r;
}
 
