
#include "hw/compat.h"
#include "hw/debug.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "hw/snd.h"
#include "res/res.h"

#include "hw/gfx.h"
#include "hw/event.h"
#include "hw/time.h"
#include "misc/test_main.h"

using namespace reaper;
using namespace reaper::hw::snd;


int test_main()
{
	char *eff[] = { 
		"own_ship_hit", "projectile_hit", 
		"cannon", "engine_behind", "engine_cockpit",
		"explosion_large", "explosion_small", "lock_off",
		"lock_on", "menu_press", "menu_select", "missile",
		"own_ship_hit", "wind", 0};
	SoundSystem ss;
	ss.init();

	hw::snd::Camera cam(Point(5, 0, 0), Vector(0, 0, 1), Vector(0, 1, 0), 70, 70);
	ss.set_listener(cam, Vector(-52.421562,-0.000001, 0.000002));

	EffectPtr efftbl[2 * sizeof(eff) / sizeof(char*)];
	int n;
/*
	for (n = 0; eff[n] != 0; n+=1) {
		derr << "loading: " << eff[n] << '\n';
		efftbl[n] = ss.prepare_effect(eff[n]);
	}
	for (int i = 0; i < n; i++) {
		efftbl[i]->set_position(Point(5, 2, 0));
		derr << "play: " << eff[i] << '\n';
		efftbl[i]->play();
		hw::time::msleep(1000);
		efftbl[i]->stop();
	}
	hw::time::msleep(5000);
*/
	derr << "engine, cockpit\n";
	EffectPtr eng_cockpit = ss.prepare_effect("engine_cockpit");
	eng_cockpit->set_loop();
	eng_cockpit->set_velocity(Vector(-51.554909, -0.000001, 0.000002));
	eng_cockpit->play();
	hw::time::msleep(5000);
	eng_cockpit->stop();

	hw::time::msleep(1000);

	derr << "engine, behind\n";
	EffectPtr eng_behind = ss.prepare_effect("engine_behind");
	eng_behind->set_loop();
	eng_behind->play();
	hw::time::msleep(5000);
	eng_behind->stop();

	return 0;
}



