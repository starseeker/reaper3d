
#include "hw/compat.h"

#include "misc/test_main.h"
#include "hw/video.h"
#include "hw/gfx.h"
#include "hw/gl.h"

#include <fstream>

using namespace reaper;


int test_main()
{
	hw::gfx::Gfx gx;
	hw::video::Player* player = new hw::video::Player(gx, "mel");

	player->initgfx();
	player->play();
	return 0;
}
 

