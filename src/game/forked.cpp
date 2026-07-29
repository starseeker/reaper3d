

#include <atomic>
#include <memory>

#include "hw/gfx.h"
#include "hw/concurrent.h"
#include "game/menus.h"
#include "game/helpers.h"
#include "res/res.h"
#include "game/forked.h"

namespace reaper {

class Reaper::Run
{
	std::atomic_bool quit_req{false};
	std::atomic_bool done{false};
	hw::concurrent::Semaphore init_done;
	std::string error;
public:
	void run()
	{
//		debug::debug_disable();
		debug::debug_priority(-1);
		
		try {
			std::unique_ptr<hw::gfx::Gfx> gx = std::make_unique<hw::gfx::Gfx>();
			Args args;
			std::unique_ptr<game::Game> go = std::make_unique<game::Game>(*gx, args);
			init_done.signal();
			go->game_init();
			go->game_start();
			while (!(quit_req.load() || go->loop_once()))
				;
			done.store(true);
			return;
		}
		catch (const error_base& e) {
			error = e.what();
		}
		catch (const std::exception& e) {
			error = e.what();
		}
		catch (...) {
			error = "unknown error\n";
		}
		done.store(true);
		init_done.signal();
	}
	void start_wait()
	{
		init_done.wait();
	}
	void req_stop()
	{
		quit_req.store(true);
	}
	std::string get_errmsg()
	{
		return error;
	}
	bool is_done()
	{
		return done.load();
	}
};

Reaper::Reaper(const std::string& root)
 : run(std::make_unique<Run>())
{
	if (!root.empty()) {
		res::add_datapath(root + "/data/");
	}
}

Reaper::~Reaper()
{
	stop();
}

bool Reaper::start()
{
	game_thread = std::thread(&Run::run, run.get());
	run->start_wait();
	return run->get_errmsg().empty();
}

void Reaper::stop()
{
	if (!run)
		return;

	run->req_stop();
	if (game_thread.joinable())
		game_thread.join();
	run.reset();
}

bool Reaper::is_done()
{
	return !run || run->is_done();
}

std::string Reaper::get_errmsg()
{
	return run ? run->get_errmsg() : std::string{};
}

}


extern "C"
reaper::Reaper* create_reaper(void*) {
	return new reaper::Reaper();
}
