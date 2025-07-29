
#include "hw/compat.h"

#include "hw/debug.h"
#include "hw/gfx.h"
#include "hw/snd.h"
#include "hw/hwtime.h"

#include "misc/test_main.h"

#include <iostream>

using namespace reaper;

int test_main()
{
//	hw::gfx::Gfx gx;
//	gx->change_mode(hw::gfx::VideoMode(640, 480));
	hw::snd::SoundSystem ss;

	hw::snd::Music* mp3 = ss.prepare_music("test");

	mp3->play();

	hw::time::TimeSpan now = hw::time::get_time();
	while (! exit_now()) {
		hw::time::msleep(100);
//			ss.do_stuff();
	}
}


