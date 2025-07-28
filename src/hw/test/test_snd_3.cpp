
/*
 * $Author: peter $
 * $Date: 2001/08/20 16:59:40 $
 * $Log: test_snd_3.cpp,v $
 * Revision 1.2  2001/08/20 16:59:40  peter
 * *** empty log message ***
 *
 * Revision 1.1  2001/07/27 15:47:37  peter
 * massive reorg...
 *
 * Revision 1.2  2001/04/18 16:25:47  henrik
 * lade till explosionsljud vid död...
 *
 * Revision 1.1  2001/04/11 14:44:01  peter
 * ljudfixar
 *
 *
 */

#include "hw/compat.h"

#include "hw/debug.h"
#include "hw/snd.h"
#include "hw/time.h"
#include "hw/gfx.h"
#include "hw/event.h"
#include "gfx/camera.h"
#include "hw/worker.h"

#include <iostream>
#include "misc/test_main.h"

using namespace reaper;
using namespace reaper::hw::snd;

namespace id = hw::event::id;

class Foo : public hw::worker::Job
{
	bool operator()() { return true; }
};

int test_main()
{
	hw::gfx::Gfx gx;
	hw::worker::worker()->add_job(new Foo);
	gx.change_mode(hw::gfx::VideoMode(200, 100));
	hw::event::EventSystem es(gx);
	hw::event::EventProxy ep = hw::event::EventSystem::get_ref(0);
	SoundSystem ss;
	ss.init();

	Effect* eff1 = ss.prepare_effect("missile");
	Effect* eff2 = ss.prepare_effect("explosion_large");
	eff2->set_loop();

	Camera cam(Point(0,0,0), Vector(0,0,-1), Vector(0,1,0), 90, 90);

	eff1->set_position(Point(-5, 0, 0));
	eff2->set_position(Point( 5, 0, 0));

	while (!exit_now()) {
		if (ep.key(hw::event::id::Escape))
			break;
		if (ep.key('J'))
			eff1->play();
		if (ep.key('M'))
			eff1->stop();
		if (ep.key('S'))
			eff2->play();
		if (ep.key('X'))
			eff2->stop();

		if (ep.key(id::Left))
			cam.pos.x -= 10;
		if (ep.key(id::Right))
			cam.pos.x += 10;
		if (ep.key(id::Up))
			cam.pos.z -= 10;
		if (ep.key(id::Down))
			cam.pos.z += 10;

		ss.set_camera(cam);
		ss.do_stuff();
		hw::time::msleep(100);
		gx.update_screen();
	}
	return 0;
}

