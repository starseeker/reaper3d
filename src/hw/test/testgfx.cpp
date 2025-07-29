#include "hw/compat.h"

#include <stdio.h>

#include "hw/gl.h"

#include "hw/gfx.h"
#include "hw/hwtime.h"
#include "hw/debug.h"

#include "misc/test_main.h"

using reaper::hw::gfx::Gfx;
using reaper::hw::time::msleep;


int test_main()
{
	Gfx gx;	// Get a handle to gfx-system
	reaper::debug::debug_disable("gfx_x11");
	// Do something...
	glViewport(0, 0, gx.current_mode().width, 
			 gx.current_mode().height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90,1,1,20);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0,0,-3, 0,0,0, 0,1,0);

	glColor3f(1.0, 1.0, 1.0);
	gluSphere(gluNewQuadric(), 1, 20, 50);
	int i = 100;
	while (! exit_now() || i-- < 0) {

		// Draw it!
		gx.update_screen();
		msleep(100);
	}
	return 0;
}

