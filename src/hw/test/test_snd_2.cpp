
/*
 * $Author: henrik $
 * $Date: 2001/04/18 16:25:47 $
 * $Log: test_snd_2.cpp,v $
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
#include "hw/hwtime.h"
#include "hw/gfx.h"
#include "hw/event.h"

#include <iostream>
#include "misc/test_main.h"

using namespace reaper;
using namespace reaper::hw::snd;


int test_main()
{
	hw::gfx::Gfx gx;
	gx->change_mode(hw::gfx::VideoMode(200, 100));
	hw::event::EventSystem es(gx);
	hw::event::EventProxy ep = hw::event::EventSystem::get_ref(0);
	SoundSystem ss;
	ss.init();
	Music* mp3 = ss.prepare_music("reaper");
	Effect* eff1 = ss.prepare_effect("missile");
	Effect* eff2 = ss.prepare_effect("explosion_large");
	eff2->set_loop();
	//mp3->play();

	while (!exit_now()) {
		if (ep.key(hw::event::id::Escape))
			break;
		if (ep.key('J'))
			eff1->play();
		if (ep.key('S'))
			eff2->play();
		if (ep.key('X'))
			eff2->stop();
		if (ep.key('M'))
			eff1->stop();
		ss.do_stuff();
		hw::time::msleep(10);
		gx->update_screen();
	}
	mp3->stop();
	return 0;
}

