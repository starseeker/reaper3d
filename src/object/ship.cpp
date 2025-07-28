

#include "hw/compat.h"

#include <string>

#include "object/base.h"
#include "object/factory.h"
#include "object/hull.h"
#include "object/ai.h"
#include "object/player_ai.h"
#include "object/sound.h"
#include "object/collide.h"
#include "object/weapon.h"
#include "object/current_data.h"
#include "object/phys.h"
#include "object/phys_data.h"
#include "object/renderer.h"
#include "object/controls.h"
#include "object/object_impl.h"
#include "res/config.h"
#include "main/types_io.h"
#include "hw/event.h"

#include "object/object.h"

namespace reaper {
namespace object {



class ShipFighter : public MixinAI<GenShip, ai::ShipFighter>
{
public:
	ShipFighter(MkInfo mk)
	 : MixinAI<GenShip, ai::ShipFighter>(mk, 
	 	new ai::ShipFighter(data, dyn_data.velocity, ctrl))
	{ }
};

class ShipBomber : public MixinAI<GenShip, ai::ShipBomber>
{
public:
	ShipBomber(MkInfo mk)
	 : MixinAI<GenShip, ai::ShipBomber>(mk, 
	 	new ai::ShipBomber(data, dyn_data.velocity, ctrl))
	{ }
};

class ShipPlayer : public MixinAI<GenShip, ai::PlayerControl>
{
public:
	ShipPlayer(MkInfo mk)
	 : MixinAI<GenShip, ai::PlayerControl>(mk, new ai::PlayerControl(ctrl, mk.id))
	{ }

};



SillyBase* ship(MkInfo mk)
{
	char t = (mk.name.size() == 5) ? 'n' : mk.name[5];
	std::string name = mk.name.substr(0, 5);
	MkInfo mk2(name, mk.mtx, mk.cid, mk.id, factory::inst().info(mk.name, name), mk.cdata);
	switch (t) {
	case 'n': return new ShipFighter(mk2);
	case 'b': return new ShipBomber(mk2);
	case 'P': return new ShipPlayer(mk2);
	default: return 0;
	}
}


namespace {
struct Foo {
	Foo() {
		factory::inst().register_object("ship2", ship);
		factory::inst().register_object("ship3", ship);
		factory::inst().register_object("ship4", ship);
		factory::inst().register_object("shipX", ship);

		factory::inst().register_object("ship2b", ship);
		factory::inst().register_object("ship3b", ship);
		factory::inst().register_object("ship4b", ship);
		factory::inst().register_object("shipXb", ship);

		factory::inst().register_object("ship2P", ship);
		factory::inst().register_object("ship3P", ship);
		factory::inst().register_object("ship4P", ship);
		factory::inst().register_object("shipXP", ship);

	}
} bar;
}

void ship() { }

}
}

