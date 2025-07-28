
#ifndef REAPER_MISC_MONITOR_H
#define REAPER_MISC_MONITOR_H

#include <deque>
#include <algorithm>
#include "misc/sequence.h"
#include "misc/stlhelper.h"
#include "misc/command.h"

namespace reaper {
namespace misc {

template<class T>
class Ping {
	T val;
public:
	Ping(const T& v) : val(v) { }
#ifdef MSVC
	void operator()(ick<1, T>::VCommand<void>& cmd) {
		cmd(val);
	}
#else
	void operator()(Command<void, 1, T>& cmd) {
		cmd(val);
	}
#endif
};


template<class T>
class Monitored
{
#ifdef MSVC
	typedef ick<1, T>::VCommand<void> Cmd;
#else
	typedef Command<void, 1, T> Cmd;
#endif
	T val;
	std::deque<Cmd> listeners;
	Monitored(const Monitored<T>&);
public:
	Monitored(const T& init) : val(init) { }
	~Monitored() {
//		for_each(seq(listeners), std::mem_fun_ref(&Cmd::del));
	}
	void onchange(const Cmd& cmd) {
		listeners.push_back(cmd);
	}
	Monitored<T>& operator=(const T& v)
	{
		if (val != v) {
			val = v;
			for_each(seq(listeners), Ping<T>(val));
		}
		return *this;
	}
//	operator T() const { return val; }
	operator const T&() const { return val; }
};

class Switch
{
	bool status;
#ifdef MSVC
	typedef ick<0>::VCommand<bool> Cmd;
#else
	typedef Command<bool, 0> Cmd;
#endif
	Cmd cmd;
	Switch(const Switch&);
public:
	Switch() : status(false) { }
	Switch(bool b) : status(b) { }
	void set(Cmd c) {
		cmd = c;
	}
	void on() {
		cmd();
		status = true;
	}
	void off() {
		status = false;
	}
	Switch& operator=(bool sw)
	{
		sw ? on() : off();
		return *this;
	}
	operator bool() const
	{
		return status;
	}
	// FIXME
	operator const bool&() const
	{
		return status;
	}
};

}
}

#endif



