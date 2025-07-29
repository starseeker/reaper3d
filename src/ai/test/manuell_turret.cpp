
#include "hw/compat.h"
#include "gfx/gfx.h"
#include "hw/gfx.h"
#include "hw/event.h"
#include "hw/hwtime.h"
#include "hw/mapping.h"
#include "world/world.h"
#include "gfx/main.h"
#include "gfx/object.h"
#include "gfx/object_manager.h"
#include "object/object.h"
#include "hw/gl.h"
#include "hw/debug.h"
#include "phys/engine.h"
#include <iostream>
#include "net/game_mgr.h"

using namespace reaper;

using object::SillyData;
using object::SillyTest;
using object::Ship;
using object::DummyTurret;
using object::Turret;

using hw::time::timespan;

using hw::event::EventProxy;
using hw::event::EventSystem;
using net::player_id;

using phys::Engine;

namespace key = hw::event::key;

namespace { debug::DebugOutput derr("test_input"); }

class TestTurret : public object::Turret
{
	player_id my_id;
public:

	TestTurret(player_id id, const SillyData &d)
	 : my_id(id), Turret(d) { }
	
	void think() {
		
		static EventProxy ep = EventSystem::get_ref(my_id);
		
		if (ep.key(key::Left)) {
			ctrl.yaw+=1;
		}
		if (ep.key(key::Right)) {
			ctrl.yaw-=1;
		}
		if (ep.key(key::Down)) {
			ctrl.pitch-=1;
		}
		if (ep.key(key::Up)) {
			ctrl.pitch+=1;
		}
		if (ep.key(' '))
			ctrl.fire = true;
		else
			ctrl.fire = false;
	}
//	void collide(object::CollisionInfo&) { }
};

using reaper::hw::event::Event;

class esc_map
 : public reaper::hw::event::Mapping
{
public:
	virtual Event operator()(key::Keycode key, bool pressed)
	{
		Event e(key, pressed);
		if (key != hw::event::key::Escape)
			e.recv = 1;
		if (key < 128)
			e.key = (key::Keycode)toupper(key);
		return e;
	}
	virtual Event operator()(int axis, float val)
	{
		if (axis >= 2)
			axis -= 2;
		Event e(axis, val);
		e.recv = 1;
		return e;
	}
};


int main()
{      
	debug::DebugOutput dout("test_input",0);
	int* i = new int;
        try {

		hw::gfx::Gfx gx;

		world::WorldRef wr = world::World::create("test_level");
		net::GameMgr game;
		int id = game.start_singleplayer();


                // Get a handle to the event-system
		EventSystem es(gx);
		es.set_local_player(1);
		EventProxy ep = EventSystem::get_ref(0);
		es.set_mapping(hw::event::create_mapping(esc_map()));

                
                // Initialize graphics settings	
                gfx::Settings s;
                s.width = gx->current_mode().width;
                s.height = gx->current_mode().height;

                // Get interface to a renderer
		gfx::RendererRef rr = gfx::Renderer::create(s);
		gfx::object::ManagerRef mr = gfx::object::Manager::create();
	
		dout << "Constructing objects...\n";
		Point p(0,800,0);
		TestTurret *turret1 = new TestTurret(1, SillyData(None,Matrix4(p)));
		wr->add_object(turret1);
		
		SillyTest *obj = new SillyTest(SillyData(None,Matrix4(p + Point(10,0,0))),1,"ammo_box");		
		wr->add_object(obj);

		reaper::phys::Engine phys_en;

                timespan prevtime = hw::time::get_time();

                while (true) {
                        static double time = 0;

                        // Wait for event and exit when escape is pressed
                        if (ep.key(hw::event::key::Escape))
                                break;

                        double timediff = static_cast<double>(hw::time::get_time() - prevtime) * 1e-6;
                        prevtime = hw::time::get_time();

			turret1->think();
			
			phys_en.update_world(time,timediff);
			
	                gfx::Camera c(p + Point(0,10,30), p, Vector(0,1,0),70);

                        rr->render(c);
                        gx->update_screen();

                        if((time += timediff) > 5000)
                                break;
                }

                return 0;
        }
        catch(const fatal_error_base &e) {
                dout << e.what() << endl;
                return 1;
        } 
        catch(const error_base &e) {
                dout << e.what() << endl;
                return 1;
        }
        catch(exception &e) {
                dout << "Std exception: " << e.what() << endl;
                return 1;
        }
}
 
