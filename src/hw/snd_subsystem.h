#ifndef REAPER_HW_SND_SUBSYSTEM_H
#define REAPER_HW_SND_SUBSYSTEM_H

#include <map>
#include "main/types.h"
#include "hw/snd.h"

namespace reaper
{
namespace hw
{
namespace snd
{

class Subsystem
{
public:
	virtual bool init() = 0;
	virtual void set_listener(const Point& pos, const Vector& dir, const Vector& vel) = 0;
	virtual void set_volume(float vol) = 0;

	virtual EffectPtr prepare(AudioSourcePtr) = 0;
	virtual SoundPtr prepare_streaming(AudioSourcePtr) = 0;

	virtual void do_stuff() { }
	virtual ~Subsystem() { }
};


}
}
}

#endif

