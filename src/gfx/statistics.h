#ifndef REAPER_GFX_STATISTICS_H
#define REAPER_GFX_STATISTICS_H

#include "hw/profile.h"

#include "gfx/displaylist.h"
#include "misc/map.h"

namespace reaper {
namespace gfx {
namespace lowlevel {

enum Profiles {
	Profiles_Begin = 0, 
	Sky_prf = 0, Terrain_prf, Water, ObjectPrepare, ObjectRender,
	EffectRender, Light, Shadow, HUDprf, Clear, Simul, Cubemap,
	Profiles_End
};						

enum Counters {
	Counters_Begin = 0, 
	Sillys = 0, Statics, Dynamics, Shots, Effects, Shadows,
	Counters_End
};

class Statistics 
{
	misc::DisplayList proj_setup, proj_restore;
	TextureRef tr;
	MeshRef mr;
	LightRef lr;
	Renderer *r;
public:
	typedef reaper::misc::Map<Counters, std::pair<int, std::string> > CntMap;
	typedef reaper::misc::Map<Profiles, hw::time::Profiler2>     PrfMap;

	CntMap cnt;
	PrfMap prf;

	Statistics(Renderer *r);
	void reset();
	void setup();
	void restore();
	void render();

	hw::time::Profiler2& operator[](Profiles p) { return prf[p]; }
	int& operator[](Counters c) { return cnt[c].first; }
};

}
}
}

#endif

/*
 * $Author: fizzgig $
 * $Date: 2002/05/16 23:58:59 $
 *
 */
