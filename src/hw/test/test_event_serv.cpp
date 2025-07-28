
#include "hw/compat.h"

#include <string>
#include <list>
#include <vector>

#include "hw/debug.h"
#include "hw/gfx.h"
#include "hw/event.h"
#include "hw/time.h"
#include "main/types.h"

#include "net/game_mgr.h"

#include "misc/test_main.h"

using namespace reaper;


using hw::gfx::Gfx;
using hw::event::Event;
using hw::event::EventSystem;
using hw::event::EventQueue;
using hw::event::EventProxy;
using net::GameMgr;

namespace id = hw::event::id;

class test_map
 : public hw::event::Mapping
{
public:
	virtual Event operator()(const Event& ev)
	{
		Event e(ev);
		if (e.id != id::Escape
		 && (e.id < id::F1 || e.id > id::F12))
			e.recv = hw::event::Player;
		if (e.id < 128)
			e.id = id::mk(toupper(e.id));
		return e;
	}
};

const bool run_multi = true;

int test_main()
{
	debug::debug_disable("sockstream");

	Gfx gx;
	gx.change_mode(hw::gfx::VideoMode(400, 300));
	EventSystem ss(gx);
	ss.set_mapping(new test_map());
	GameMgr game(gx);

	hw::event::PlayerID local_id = run_multi
		? game.init_multiplayer()
		: game.init_singleplayer();

	if (! game.start_req()) {
		derr << "start failed\n";
		exit(1);
	}
	game.wait_for_start();

	EventProxy sys = EventSystem::get_ref(0);
	EventProxy ply = EventSystem::get_ref(local_id);

	while (! exit_now()) {
		Event e;
		while (sys.get_event(e)) {
			derr << "sys: " << e << '\n';
			if (e.id == id::Escape)
				return 0;
		}
		while (ply.get_event(e)) {
			derr << "ply: " << e << '\n';
		}
		hw::time::msleep(10);
		gx.update_screen();
	}
	return 0;
}



