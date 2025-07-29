#ifndef HW_SND_H
#define HW_SND_H

#include "main/types.h"
#include <memory>

namespace reaper {
namespace hw {
namespace snd {

/// Base for sounds. General settings.
class Sound
{
public:
	/// Starts playing, from the beginning.
	virtual void play() = 0;

	/// Stops playing. Can be restarted again.
	virtual void stop() = 0;

	virtual void set_volume(float vol) = 0;

	virtual ~Sound() { }
};

class Effect;
typedef std::unique_ptr<Effect> EffectPtr;
typedef std::unique_ptr<Sound> SoundPtr;

/// 3d sound effect.
class Effect
 : public Sound
{
public:
	virtual void set_position(const Point&) = 0;
	virtual void set_direction(const Vector&) = 0;
	virtual void set_velocity(const Vector&) = 0;

	/// Ratio, 1.0 is normal rate.
	virtual void set_pitch(float p) = 0;

	/// Cheap cloning, should share resources if possible.
	virtual EffectPtr clone() const = 0;

	virtual void set_loop() = 0;
};


struct SoundSystem_impl;

/** SoundSystem. Initializes sound system and loads sound/music files. */
class SoundSystem
{
	SoundSystem_impl* impl;
public:
	SoundSystem();
	~SoundSystem();

	bool init();

	/// Load and prepare sound effect.
	EffectPtr prepare_effect(const std::string& id);

	/// Load and prepare music (streamed sound).
	SoundPtr prepare_music(const std::string& id);

	/** Set global volume.
	 *  \param vol Volume (normalized 0..1)
	 */
	void set_volume(float vol);

	/// Set listener (camera) position/direction/velocity.
	void set_listener(const Point& pos, const Vector& dir, const Vector& vel);

	void do_stuff();

	/// Dummy implementation of effects, 
	std::unique_ptr<Effect> dummysound();
};


}
}
}

#endif


/*
 * $Author: pstrand $
 * $Date: 2002/05/21 10:09:28 $
 * $Log: snd.h,v $
 * Revision 1.36  2002/05/21 10:09:28  pstrand
 * *** empty log message ***
 *
 * Revision 1.35  2002/01/23 00:59:56  peter
 * soundfixes again
 *
 * Revision 1.34  2002/01/22 23:44:04  peter
 * reversed last two revs
 *
 * Revision 1.32  2002/01/17 04:58:45  peter
 * ljud-plugin-mm fix..
 *
 * Revision 1.31  2002/01/11 21:26:36  peter
 * audiosource, mm
 *
 * Revision 1.30  2002/01/07 14:00:27  peter
 * resurs och ljudmeck
 *
 * Revision 1.29  2001/11/26 02:20:08  peter
 * ljudfixar (smartptr), andra mindre fixar..
 *
 * Revision 1.28  2001/08/06 12:16:23  peter
 * MegaMerge (se strandy_test-grenen f�r diffar...)
 *
 * Revision 1.27.2.1  2001/08/03 13:43:59  peter
 * pragma once obsolete...
 *
 * Revision 1.27  2001/07/30 23:43:21  macke
 * H�pp, d� var det k�rt.
 *
 * Revision 1.26  2001/07/27 15:47:35  peter
 * massive reorg...
 *
 * Revision 1.25  2001/07/06 01:47:21  macke
 * Refptrfix/headerfilsst�d/objekt-skapande/mm
 *
 * Revision 1.24  2001/05/30 01:02:11  peter
 * ljudet funkar (bra) igen, f�r mig i alla fall =)
 *
 * Revision 1.23  2001/05/19 10:31:27  peter
 * ljudomorg..
 *
 * Revision 1.22  2001/05/03 11:03:20  peter
 * *** empty log message ***
 *
 */


