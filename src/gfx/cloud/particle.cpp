
#include "hw/compat.h"

#include "main/types.h"
#include "gfx/gfx_types.h"

#include "gfx/cloud/particle.h"

namespace reaper {
namespace gfx {
namespace cloud {

Particle::Particle(const Point& p, float r)
 : pos(p), radius(r)
{ }

Particle::Particle(const Point& p, float r, float lifetime, float alpha)
 : pos(p), radius(r)
{
	// For modernization, we'll ignore the lifetime and alpha parameters
	// since they're not stored in the current struct
}



}
}
}

