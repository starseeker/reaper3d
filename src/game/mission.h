
#ifndef REAPER_GAME_MISSION_H
#define REAPER_GAME_MISSION_H

#include <string>
#include <deque>
#include <map>
#include <memory>

#include "ai/msg.h"
#include "game/object_mgr.h"

namespace reaper {
namespace game {
namespace scenario {

class Goal {
public:
	virtual bool test(double time, ObjectMgr&, ai::MsgQueue&) = 0;
	virtual std::string next() const = 0;
	virtual ~Goal() = default;
};

std::unique_ptr<Goal> mk_goal(const std::string& goal);

struct Mission {
	std::string name;
	std::string dialog;
	std::deque<std::string> objectgroups;
	std::deque<std::unique_ptr<Goal>> goals;
};

using Missions = std::map<std::string, std::unique_ptr<Mission>>;
struct Scenario {
	Missions missions;
};

void init_mission_loader();


}
}
}

#endif
