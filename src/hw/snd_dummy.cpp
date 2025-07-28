/*
 * $Author: pstrand $
 * $Date: 2002/05/21 10:09:28 $
 * $Log: snd_dummy.cpp,v $
 * Revision 1.37  2002/05/21 10:09:28  pstrand
 * *** empty log message ***
 *
 * Revision 1.36  2002/02/21 15:26:01  peter
 * *** empty log message ***
 *
 * Revision 1.35  2002/01/31 16:10:49  peter
 * *** empty log message ***
 *
 * Revision 1.34  2002/01/21 03:02:03  peter
 * splittade time.h
 *
 * Revision 1.33  2002/01/20 16:25:43  peter
 * audiosource fix
 *
 * Revision 1.32  2002/01/17 04:58:45  peter
 * ljud-plugin-mm fix..
 *
 * Revision 1.31  2002/01/11 21:26:37  peter
 * audiosource, mm
 *
 * Revision 1.30  2002/01/07 14:00:27  peter
 * resurs och ljudmeck
 *
 * Revision 1.29  2001/12/11 22:54:27  peter
 * ladda/spara
 *
 * Revision 1.28  2001/11/26 19:35:50  peter
 * no message
 *
 * Revision 1.27  2001/11/26 02:20:08  peter
 * ljudfixar (smartptr), andra mindre fixar..
 *
 * Revision 1.26  2001/11/18 23:47:26  peter
 * fippel
 *
 * Revision 1.25  2001/11/18 16:12:28  peter
 * hmm
 *
 * Revision 1.24  2001/11/14 22:42:56  peter
 * *** empty log message ***
 *
 * Revision 1.23  2001/11/10 11:58:30  peter
 * diverse minnesfixar och strul...
 *
 * Revision 1.22  2001/08/12 13:32:26  peter
 * snd abstr...
 *
 * Revision 1.21  2001/07/27 15:47:35  peter
 * massive reorg...
 *
 * Revision 1.20  2001/07/23 23:48:10  macke
 * Slimmad grafikhantering samt lite namnbyten
 *
 */

#include "hw/compat.h"

#include <string>

#include "hw/snd.h"
#include "hw/snd_wave.h"
#include "hw/snd_mp3.h"
#include "hw/snd_subsystem.h"
#include "hw/debug.h"
#include "hw/interfaces.h"


namespace reaper
{
namespace hw
{
namespace snd
{
namespace dummy
{


class Dummy
 : public Effect
{
	std::string self;
	ifs::Snd* main;
	void msg(const std::string& s) { 
//		main->derr() << s << ": " << self << '\n';
	}
public:
	Dummy(const std::string& s, ifs::Snd* m) : self(s), main(m) {
		msg("Creating");
	}
	~Dummy() {
//		msg("dying");
	}
	void play() {
		msg("Playing");
	}
	void stop() {
		msg("Stop");
	}
	void set_volume(float) {
		msg("Setvolume");
	}
	void set_position(const Point&) {
//		msg("Setpos");
	}
	void set_direction(const Vector&) {
	//	msg("Setdir");
	}
	void set_velocity(const Vector&) {
	//	msg("Setvelocity");
	}
	void set_pitch(float) {
		msg("Setpitch");
	}
	void set_loop() {
		msg("Setloop");
	}
	void fade_in(float) {
		msg("fadein");
	}
	void fade_out(float) {
		msg("fadeout");
	}
	EffectPtr clone() const {
		return EffectPtr(new Dummy(self, main));
	}
};


class Subsystem
 : public snd::Subsystem
{
	ifs::Snd* main;
public:
	Subsystem(ifs::Snd* m) : main(m) { }
	~Subsystem() {
		main->derr() << "dummy sound shutdown\n";
	}
	bool init() { return true; }

	EffectPtr prepare(AudioSourcePtr sd)
	{
		delete sd;
		return EffectPtr(new Dummy("effect", main));
	}
	SoundPtr prepare_streaming(AudioSourcePtr sd)
	{
		delete sd;
		return SoundPtr(new Dummy("music", main));
	}
	void set_volume(float vol) { }
	void set_listener(const Point& pos, const Vector& dir, const Vector&) { }
};


}
}
}
}

extern "C" {
DLL_EXPORT void* create_snd_dummy(reaper::hw::ifs::Snd* main)
{
	return new reaper::hw::snd::dummy::Subsystem(main);
}
}

