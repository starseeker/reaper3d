
#ifndef REAPER_MISC_EVENT_H
#define REAPER_MISC_EVENT_H

#include <deque>


namespace reaper {
namespace misc {

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
		for (auto& listener : listeners)
			listener();
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

