
/* $Id: profile.h,v 1.4 2002/09/12 14:21:28 pstrand Exp $ */

#ifndef REAPER_HW_PROFILE_H
#define REAPER_HW_PROFILE_H

#include "hw/abstime.h"
#include <vector>

namespace reaper
{
namespace hw
{
namespace time
{


class Profiler
{
	std::string name;
	TimeSpan begin;
	TimeSpan total;
	int count;
	long td;
	typedef std::string (Profiler::*meth_ptr)(const std::string&) const;
	void int_report(const std::string& desc, meth_ptr m) const;
	std::string mkreport(const std::string& d, const std::string& d2, double v) const;
public:
	Profiler(const std::string& name = "");
	void start();
	long stop();
	void reset();
	int get_count() const;
	long get_last() const;
	double get_avg() const;

	/// Reports count, average and total
	std::string report(const std::string& desc = "") const;
	// total, se internal debugstream for output
	void int_report(const std::string& desc = "") const;

	/// Reports last and average
	std::string report_last(const std::string& desc = "") const;
	// Last&average, use internal debugstream for output
	void int_report_last(const std::string& desc = "") const;

	/// Reports last and average (short format)
	std::string report_last_short(const std::string& desc = "") const;
};



class Profile {
	std::string label;
	Profiler p;
	Profiler& pref;
public:
	Profile(Profiler& pp);
	Profile(const std::string& label);
	~Profile();
};

class Profiler2 : public hw::time::Profiler {
	float x,y;
	float textlen, height;
	std::vector<float> history;
	int ins;
public:

	Profiler2(const std::string& label, float x, float y, 
		  float textlen = .5, float height = 2, int len = 200);
	void draw_txt();
	void draw_txt_short();
	void draw_last(float frame_length);
	void draw_avg(float frame_length);
	void draw_history(float frame_length);
};

class Timer
{
//	TimeSpan prevtime;
	double time;
	double freq;
public:
	Timer(double tick_frequency)
	 : time(0), freq(tick_frequency)
	{ }

	bool update(double dt) {
//		double dt = (get_time().approx() - prevtime.approx()) * 1e-6;
//              prevtime = get_time();		
		time += dt;

		if(time > freq) {
			time -= freq;
			return true;
		} else {
			return false;
		}
	}
};


class Ticker
{
	TimeSpan prev;
	TimeSpan skips;

	TimeSpan diff;
	TimeSpan diff_err;
	TimeSpan ticks;

	bool paused;

public:
	Ticker() : paused(false) { reset(); }
	void reset() {
		prev = get_time();
		skips = ticks = diff = TimeSpan(0,0);
	}
	void tick(float scale = 1.0) {
		TimeSpan now = get_time();

		if (!paused) {
			diff = (now - prev);
			diff *= scale;
			ticks += diff;
		} else {
			diff = TimeSpan(0,0);
		}
		prev = now;
	}
	TimeSpan last() { return diff; }
	RelTime last_ms() {
		diff += diff_err;
		diff_err = TimeSpan(0,0);
		RelTime diff_ms = diff.approx().to_ms();
		diff_err = diff - TimeSpan::from_ms(diff_ms);
		
		return diff_ms;
	}
	TimeSpan total() { return ticks; }
	void pause(bool b) {
		paused = b;
	}
};

}
}
}
#endif

