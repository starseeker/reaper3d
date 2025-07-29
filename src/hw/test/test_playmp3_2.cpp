
/*
 * $Author: pstrand $
 * $Date: 2002/06/09 22:19:00 $
 * $Log: test_playmp3_2.cpp,v $
 * Revision 1.21  2002/06/09 22:19:00  pstrand
 * solaris
 *
 * Revision 1.20  2002/04/12 18:51:12  pstrand
 * dynobj i quadtree
 *
 * Revision 1.19  2002/01/21 00:20:59  peter
 * musikfixar, mm..
 *
 * Revision 1.18  2002/01/17 04:58:48  peter
 * ljud-plugin-mm fix..
 *
 * Revision 1.17  2002/01/11 21:26:39  peter
 * audiosource, mm
 *
 * Revision 1.16  2002/01/01 23:51:55  peter
 * ..
 *
 * Revision 1.15  2001/12/08 01:30:07  peter
 * no message
 *
 * Revision 1.14  2001/12/08 00:00:42  peter
 * mp3dekodning funkar (igen)...
 *
 * Revision 1.13  2001/12/06 20:00:06  peter
 * experimental antifunctional dump
 *
 * Revision 1.12  2001/07/15 21:42:46  peter
 * dynload, time fixar mm
 *
 *
 */

#include "hw/compat.h"

#include "hw/debug.h"
#include "hw/snd.h"
#include "hw/hwtime.h"
#include "hw/worker.h"

#include <iostream>
#include "misc/test_main.h"

using namespace reaper;
using namespace reaper::hw::snd;

int test_main()
{
	SoundSystem ss;
	ss.init();
	SoundPtr mp3 = ss.prepare_music("reaper.small");
	if (!mp3.get())
		return 1;
	mp3->play();

/*
	for (int i = 0; i < 2000; ++i) {
		ss.do_stuff();
		hw::time::msleep(50);
	}
*/
	ss.do_stuff();
	ss.do_stuff();
	ss.do_stuff();
	ss.do_stuff();
	ss.do_stuff();
	
	mp3->stop();
	hw::worker::worker()->shutdown();
	return 0;
}

