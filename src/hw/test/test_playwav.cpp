
/*
 * $Author: peter $
 * $Date: 2002/01/01 23:51:55 $
 * $Log: test_playwav.cpp,v $
 * Revision 1.6  2002/01/01 23:51:55  peter
 * ..
 *
 * Revision 1.5  2001/08/20 16:59:39  peter
 * *** empty log message ***
 *
 * Revision 1.4  2001/04/10 20:48:20  peter
 * ljudfix
 *
 * Revision 1.3  2001/04/05 10:14:53  peter
 * picon vill ha mat, s� det blir inga kommentarer... ;)
 *
 * Revision 1.2  2001/03/15 03:22:15  peter
 * win32 fixar...
 *
 * Revision 1.1  2001/03/15 00:13:07  peter
 * wav&mp3
 *
 */


#include "hw/compat.h"

#include "hw/debug.h"
#include "hw/snd_wave.h"
#include "hw/snd_types.h"
#include "hw/snd_subsystem.h"

#include "misc/plugin.h"
#include "res/res.h"
#include "misc/test_main.h"



#include <iostream>

using namespace reaper;
using namespace reaper::hw::snd;


int test_main()
{
	misc::PluginCreator<SoundDecoder, misc::UniqueObj> dec_p;
	SoundDecoder* snd_dec = dec_p.create("snd_wave");

	misc::PluginCreator<Subsystem, misc::SharedObj> snd_p;
	Subsystem* player = snd_p.create("snd_simple");

	res::res_stream snd_in(res::Sound, "missile");

	snd_dec->init(&snd_in);
	SoundData sdata.info = snd_dec->info();
	snd_dec->read(sdata.data);

	player->prepare(&sdata);
	player->play();

	return 0;
}

