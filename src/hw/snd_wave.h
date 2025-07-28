/*
 * $Author: pstrand $
 * $Date: 2002/04/16 19:58:42 $
 * $Revision: 1.13 $
 */

#ifndef REAPER_HW_SND_WAVE
#define REAPER_HW_SND_WAVE

#include <vector>
#include <iosfwd>
#include "hw/exceptions.h"

#include "hw/snd_types.h"

namespace reaper
{
namespace hw
{
namespace snd
{


class wave_format_error_t : public hw_error_t { };


class WaveDecoder : public AudioDecoder
{
	SoundInfo wave_info;
	Samples smp;
public:
	WaveDecoder();
	~WaveDecoder();
	bool init(res::res_stream*);
	AudioSourcePtr get();
};


}
}
}

#endif

