#include "hw/compat.h"

#include <stdio.h>

#include "hw/gl.h"

#include "hw/gfx.h"
#include "hw/time.h"
#include "misc/test_main.h"

#include "game/helpers.h"

using namespace reaper;


debug::DebugOutput derr("test_progbar",0);


int test_main()
{
	hw::gfx::Gfx gx;
	game::ProgressBar bar(gx, "Loading", 4);

	bar.render("test");
	hw::time::msleep(1000);
	bar.tick("one");
	hw::time::msleep(1000);
	bar.tick("two");
	hw::time::msleep(1000);
	bar.tick("three");
	hw::time::msleep(1000);
	bar.tick("five");
	hw::time::msleep(1000);

	return 0;
}


