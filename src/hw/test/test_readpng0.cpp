
#include "hw/compat.h"

#include "misc/test_main.h"
#include "hw/gfx.h"
#include "gfx/texture.h"

using namespace reaper;

int test_main()
{
	hw::gfx::Gfx gx;
	gfx::texture::Texture("main");
	return 0;
}

