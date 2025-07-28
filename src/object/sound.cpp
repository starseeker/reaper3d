/*
 * $Author: pstrand $
 * $Date: 2002/02/27 18:08:41 $
 * $Log: sound.cpp,v $
 * Revision 1.45  2002/02/27 18:08:41  pstrand
 * release-0.96
 *
 * Revision 1.44  2002/02/11 12:02:05  peter
 * handle case without sound..
 *
 * Revision 1.43  2002/01/26 11:24:19  peter
 * resourceimprovements, and minor fixing..
 *
 * Revision 1.42  2002/01/23 01:00:07  peter
 * soundfixes again
 *
 * Revision 1.41  2002/01/22 23:44:08  peter
 * reversed last two revs
 *
 * Revision 1.39  2002/01/21 20:56:47  peter
 * soundtweaking...
 *
 * Revision 1.38  2002/01/11 21:33:06  peter
 * *** empty log message ***
 *
 * Revision 1.37  2002/01/07 16:38:27  peter
 * Singalong: "Vi hatar, vi hatar vc--, vi hatar de som vc skapat har..."
 *
 * Revision 1.36  2002/01/07 14:00:32  peter
 * resurs och ljudmeck
 *
 * Revision 1.35  2002/01/01 23:31:14  peter
 * do not create if no sound
 *
 * Revision 1.34  2001/12/05 14:04:15  peter
 * matrix now in (semi)row-major
 *
 * Revision 1.33  2001/11/26 02:20:16  peter
 * ljudfixar (smartptr), andra mindre fixar..
 *
 * Revision 1.32  2001/11/18 23:47:55  peter
 * fippel
 *
 * Revision 1.31  2001/11/11 01:23:11  peter
 * minnesfixar..
 *
 * Revision 1.30  2001/11/10 11:58:34  peter
 * diverse minnesfixar och strul...
 *
 * Revision 1.29  2001/08/20 21:41:25  peter
 * aeh
 *
 * Revision 1.28  2001/08/20 17:09:35  peter
 * sndfix
 *
 * Revision 1.27  2001/08/09 21:40:14  peter
 * engine pos...
 *
 * Revision 1.26  2001/08/06 12:16:40  peter
 * MegaMerge (se strandy_test-grenen för diffar...)
 *
 * Revision 1.25.2.3  2001/08/05 14:01:33  peter
 * objektmeck...
 *
 * Revision 1.25.2.2  2001/08/04 16:14:18  peter
 * *** empty log message ***
 *
 * Revision 1.25.2.1  2001/08/02 16:29:12  peter
 * objfix...
 *
 * Revision 1.25  2001/07/30 23:43:30  macke
 * Häpp, då var det kört.
 *
 *
 */

#include "hw/compat.h"

#include "hw/debug.h"

#include "main/types.h"
#include "main/types_io.h"
#include "object/base.h"
#include "object/sound.h"
#include "object/object.h"
#include "misc/free.h"
 
#include "snd/sound_system.h"

#include "hw/snd.h"

namespace reaper
{
namespace object
{
namespace sound
{

debug::DebugOutput derr("object::sound");

typedef snd::EffectPtr EffectPtr;

class StaticImpl : public Static
{
	EffectPtr eff;
public:
	StaticImpl(EffectPtr e) : eff(e) { }
	StaticImpl(const std::string& name, const Point& pos)
	 : eff(snd::Manager::get_ref()->load(name, 1.0))
	{
		eff->set_position(pos);
	}
	void ping()
	{
		eff->play();
	}
	Base* clone() const
	{
		return new StaticImpl(eff->clone());
	}
};

Static* create_static(const std::string& name, const Point& pos)
{
	return new StaticImpl(name, pos);
}


class EngineSound
{
	EffectPtr snd_inside, snd_outside;
	bool inside;
	hw::snd::Effect& current() {
		if (inside)
			return *snd_inside;
		else
			return *snd_outside;
	}
public:
	EngineSound() : snd_inside(0), snd_outside(0), inside(false) { }
	EngineSound(const Point& pos)
	{
		snd::SoundRef si = snd::Manager::get_ref();
		snd_inside = EffectPtr(si->load("engine_cockpit", 0.1, true));
		snd_outside = EffectPtr(si->load("engine_behind", 0.1, true));
		inside = false;
		current().play();
	}
	~EngineSound()
	{
	}
	EngineSound* clone() const
	{
		EngineSound* e = new EngineSound();
		e->snd_inside = EffectPtr(snd_inside->clone());
		e->snd_outside = EffectPtr(snd_outside->clone());
		e->inside = inside;
		return e;
	}
	void set_view(bool is_inside)
	{
		if (inside == is_inside)
			return;
//		derr << "change view: " << is_inside << '\n';
		current().stop();
		inside = is_inside;
		current().play();
	}
	void position(const Point& p)
	{
		current().set_position(p);
	}
	void velocity(const Vector& v)
	{
		current().set_velocity(v);
	}
	void pitch(float val)
	{
		current().set_pitch(1 + val*2);
	}
};


class ShipImpl : public Ship
{
	bool sound_loaded;
	bool inside;
	EngineSound* engine_snd;
public:
	ShipImpl(EngineSound* e)
	 : sound_loaded(false),
	   inside(false),
	   engine_snd(e)
	{ }
	ShipImpl(const std::string& name, const Matrix& mat, const Vector& v)
	 : sound_loaded(false), 
	   inside(false),
	   engine_snd(0)
	{
		engine_snd = new EngineSound(mat.pos());
	}
	~ShipImpl()
	{
		misc::zero_delete(engine_snd);
	}

	Base* clone() const
	{
		if (engine_snd == 0)
			return 0;
		return new ShipImpl(engine_snd->clone());
	}

	void set_location(const Matrix& mat)
	{
		if (engine_snd == 0)
			return;
		engine_snd->position(mat.pos());
	}

	void set_velocity(const Vector& vel)
	{
		if (engine_snd == 0)
			return;
//		engine_snd->velocity(vel);
	}

	void engine_pitch(float f)
	{
		if (engine_snd == 0)
			return;
		engine_snd->pitch(f / 10);
	}

	void inside_view(bool b)
	{
		if (inside == b)
			return;
		inside = b;
//		derr << "view: " << b << '\n';
		if (engine_snd != 0)
			engine_snd->set_view(b);
	}
	void kill()
	{
		misc::zero_delete(engine_snd);
	}
};


Ship* create_ship(const std::string& name, const Matrix& mat, const Vector& vel)
{
	if (snd::Manager::get_ref().valid())
		return new ShipImpl(name, mat, vel);
	else
		return 0;
}



class ProjectileImpl : public Projectile
{
	snd::EffectPtr snd;
public:
	ProjectileImpl(snd::EffectPtr s) : snd(s) { }
	ProjectileImpl(snd::EffectPtr s, const Matrix& mat, const Vector& v)
	 : snd(s)
	 {
		snd->set_position(mat.pos());
		snd->set_velocity(v);
		snd->play();
	 }

	~ProjectileImpl() {
	}
	Base* clone() const
	{
		return new ProjectileImpl(snd->clone());
	}
	void set_location(const Matrix& mat)
	{
		snd->set_position(mat.pos());
	}
	void set_velocity(const Vector& vel)
	{
		snd->set_velocity(vel);
	}
};

class DummyImpl : public Projectile
{
public:
	Base* clone() const { return new DummyImpl(); }
	void set_location(const Matrix&) { }
	void set_velocity(const Vector&) { }
};


Projectile* create_proj(const std::string& name, const Matrix& mat, const Vector& vel)
{
	if (snd::Manager::get_ref().valid()) {
		snd::EffectPtr snd = snd::Manager::get_ref()->load(name, 0.5, true);
		if (snd.get())
			return new ProjectileImpl(snd, mat, vel);
		else
			return new DummyImpl();
	} else {
		return 0;
	}
}



}
}
}

