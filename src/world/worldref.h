#ifndef REAPER_WORLD_WORLDREF_H
#define REAPER_WORLD_WORLDREF_H

#include "misc/uniqueptr.h"
#include "misc/smartptr.h"

namespace reaper {

namespace world {
	class World;
	typedef reaper::misc::UniquePtr<World> WorldRef;

}
}


#endif
