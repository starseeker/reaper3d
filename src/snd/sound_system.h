
/* $Id: sound_system.h,v 1.33 2002/05/21 10:09:47 pstrand Exp $ */


#ifndef SND_SOUND_SYSTEM_H
#define SND_SOUND_SYSTEM_H

#include "hw/snd.h"
#include "misc/uniqueptr.h"
#include "misc/smartptr.h"

namespace reaper
{
namespace object { class SillyBase; }
namespace sound
{

enum ID {
	Cannon,
	Engine_Behind, Engine_Cockpit,
	Explosion_Larger, Explosion_Large, Explosion_Small,
	Lock_Off, Lock_On, Menu_Press, Menu_Select,
	Missile, Missile_Heavy, Missile_Light, Laser, Laser_Hit, Projectile_Hit, Own_Ship_Hit, Wind
};

class Internal;

typedef hw::snd::EffectPtr EffectPtr;
typedef misc::SmartPtr<hw::snd::Effect> EffectSPtr;

class Manager;
typedef misc::UniquePtr<Manager> SoundRef;



class Manager
{
	Internal* snd_int;
	Manager();
	friend class misc::UniquePtr<Manager>;
public:
	static SoundRef get_ref();
	static SoundRef create();

	~Manager();

	void set_camera(const Point& pos, const Vector& dir, const Vector& vel);
	void start();
	void run();

	// just load, moving sound
	EffectPtr load(const std::string& id, float vol = 1.0, bool loop = false);
	// just load, positioned sound
	EffectPtr load(const std::string& id, const Point& pos, float vol = 1.0);

	// do-it-all function...
	void play(const std::string& id, const Point& pos, float vol = 1.0);

	void shutdown();
};


}
}

#endif

