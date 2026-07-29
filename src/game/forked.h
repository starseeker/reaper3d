
#ifndef REAPER_GAME_FORKED_H
#define REAPER_GAME_FORKED_H

#include <memory>
#include <string>
#include <thread>

namespace reaper {

class Reaper
{
	class Run;
	std::unique_ptr<Run> run;
	std::thread game_thread;
public:
	explicit Reaper(const std::string& rootdir = "");
	~Reaper();
	Reaper(const Reaper&) = delete;
	Reaper& operator=(const Reaper&) = delete;

	bool start();
	std::string get_errmsg();
	void stop();
	bool is_done();
};

}


extern "C" reaper::Reaper* create_reaper(void*);

#endif
