
/* $Id: gfx_types.h,v 1.32 2002/09/23 12:08:43 fizzgig Exp $ */


#ifndef HW_GFX_TYPES_H
#define HW_GFX_TYPES_H



#include <queue>

#include "hw/gfx.h"

namespace reaper {
namespace hw {
namespace lowlevel {

struct Gfx_driver_data {
	std::queue<std::pair<int,bool> > key_queue;
	gfx::VideoMode desktop;
	gfx::VideoMode current;

	bool window_active;
	bool is_accelerated;
	bool alpha;
	bool stencil;

	Gfx_driver_data()
	 : desktop(),
	   current(),
	   window_active(false),
	   is_accelerated(false),
	   alpha(false),
	   stencil(false)
	{ }
};


}
}
}


#endif
