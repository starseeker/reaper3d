
#include "hw/compat.h"

#include "object/event.h"
#include "world/world.h"
#include "phys/engine.h"

namespace reaper {
namespace object {
namespace event {

class OnDeath
{
	object::ID id;
public:
	OnDeath(object::ID i) : id(i) { }
	void operator()() {
		world::World::get_ref()->erase(id);
		reaper::phys::Engine::get_ref()->remove(id);
	}
};

misc::Command<void> mk_rm_on_death(object::ID id) {
	return misc::mk_cmd<void>(OnDeath(id));
}

}
}
}
