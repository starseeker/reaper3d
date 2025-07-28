/*
 * $Author: pstrand $
 * $Date: 2002/04/29 21:19:00 $
 * $Log: test_decodemp3.cpp,v $
 * Revision 1.2  2002/04/29 21:19:00  pstrand
 * *** empty log message ***
 *
 * Revision 1.1  2002/01/01 23:51:54  peter
 * ..
 *
 *
 */

#include "hw/compat.h"

#include <iostream>

#include "hw/snd_types.h"
#include "misc/test_main.h"
#include "misc/plugin.h"
#include "res/res.h"
#include "hw/snd_wave.cpp"

namespace reaper {
namespace hw {
namespace snd {
void put(std::ostream& os, const SoundInfo& si, int size);
void put(std::ostream& os, const Samples& si);
}
}
}

using namespace reaper;
using namespace reaper::hw::snd;
typedef misc::Plugin<AudioDecoder, void*> SndDecPlugin;

int test_main()
{
	
	SndDecPlugin snd_load;

	AudioDecoder* mp3_dec = snd_load.create("snd_mp3", 0);
	res::res_stream inp(res::Music, "reaper");
	res::res_out_stream out(res::Sound, "test_decode");

	mp3_dec->init(&inp);
	AudioSource* src = mp3_dec->get();
	put(out, src->info(), 10000);

	Samples smp;
	while (src->read(smp)) {
		put(out, smp);
	}
	
	return 0;
}

