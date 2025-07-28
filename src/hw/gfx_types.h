
/* $Id: gfx_types.h,v 1.32 2002/09/23 12:08:43 fizzgig Exp $ */


#ifndef HW_GFX_TYPES_H
#define HW_GFX_TYPES_H



#include <queue>

#include "hw/gfx.h"

struct _XDisplay;
typedef struct _XDisplay Display;

namespace reaper {
namespace hw {
namespace lowlevel {

struct Gfx_driver_data {
	Display* display;
	
	std::queue<std::pair<int,bool> > key_queue;
	gfx::VideoMode desktop;
	gfx::VideoMode current;

	bool window_active;
	bool is_accelerated;
	bool alpha;
	bool stencil;
};


}
}
}


#endif


