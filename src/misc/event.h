
#ifndef REAPER_MISC_EVENT_H
#define REAPER_MISC_EVENT_H

#include <deque>


namespace reaper {
namespace misc {

struct LocalPing
{
	template<class T>
	void operator()(T t)
	{
		t();
	}
};

template<class Cmd>
class Event
{
	class NotifyHandle
	{
		Event* event;
	public:
		NotifyHandle(Event* e)
		 : event(e)
		{ }
		void send()
		{
			event->ping_all();
		}

	};

	typedef std::deque<Cmd> CmdQ;
	CmdQ listeners;
public:
	void ping_all()
	{
		for_each(seq(listeners), LocalPing());
	}
//public:
	void add_listener(Cmd cmd)
	{
		listeners.push_back(cmd);
	}
	NotifyHandle notify()
	{
		return NotifyHandle(this);
	}
	void clear()
	{
		listeners.clear();
	}
};



}
}

#endif


