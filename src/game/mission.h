
#ifndef REAPER_GAME_MISSION_H
#define REAPER_GAME_MISSION_H

#include <string>
#include <deque>
#include <map>

#include "ai/msg.h"
#include "game/object_mgr.h"

namespace reaper {
namespace game {
namespace scenario {

class Goal {
public:
	virtual bool test(double time, ObjectMgr&, ai::MsgQueue&) = 0;
	virtual std::string next() const = 0;
	virtual ~Goal();
};

Goal* mk_goal(const std::string& goal);

struct Mission {
	std::string name;
	std::string dialog;
	std::deque<std::string> objectgroups;
	std::deque<Goal*> goals;

	Mission();
	~Mission();
};

typedef std::map<std::string, Mission*>  Missions;
struct Scenario {
	Missions missions;

	Scenario();
	~Scenario();
};

void init_mission_loader();


}
}
}

#endif

