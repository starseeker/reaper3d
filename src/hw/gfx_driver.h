
/* $Id: gfx_driver.h,v 1.6 2001/10/10 02:01:12 peter Exp $ */


#ifndef REAPER_HW_GFX_DRIVER_H
#define REAPER_HW_GFX_DRIVER_H

namespace reaper {
namespace debug { class DebugOutput; }
namespace hw {
namespace gfx { class VideoMode; }

namespace lowlevel {

struct Gfx_driver_data;

class Gfx_driver
{

public:
	virtual Gfx_driver_data* internal_data() = 0;
	virtual void update_screen() = 0;
	virtual bool setup_mode(const gfx::VideoMode&) = 0;
	virtual void restore_mode() = 0;
	virtual ~Gfx_driver() { }
};


}
}
}


#endif


