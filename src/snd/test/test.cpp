#include "hw/compat.h"
#include "hw/snd.h"
//#include "res/res.h"

using namespace reaper;
using namespace reaper::hw::snd;

void main()
{
	SoundSystem ss;

	Effect* eff1 = ss.PrepareEffect("test1");
	eff1->play();
	getchar();
}
