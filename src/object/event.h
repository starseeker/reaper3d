
#ifndef REAPER_OBJECT_EVENT_H
#define REAPER_OBJECT_EVENT_H

#include "misc/command.h"
#include "misc/event.h"
#include "object/baseptr.h"

namespace reaper {
namespace object {
namespace event {

class CollisionInfo;

class Events
{
public:
	misc::Event<misc::Command<void> > death;
};

misc::Command<void> mk_rm_on_death(object::ID id);

}
}
}

#endif

