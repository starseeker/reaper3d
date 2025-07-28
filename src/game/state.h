#ifndef REAPER_GAME_STATE_H
#define REAPER_GAME_STATE_H

#include <iosfwd>
#include <string>
#include <memory>

namespace reaper {

namespace res {
	class ConfigEnv;
	class res_out_stream;
}

namespace object { class ObjectGroup; }

namespace game {
namespace state {

typedef res::ConfigEnv Env;

class Dump {
public:
	virtual void dump(Env&) const = 0;
	virtual ~Dump() { }
};

/*class RestoreState {
public:
	virtual void restore(std::istream&) = 0;
};
*/
/*class PersistentState : public DumpState, public RestoreState
{ };
*/
typedef Dump Persistent;

class StateMgr
{
public:
	virtual void dump(const std::string& fn) const = 0;
	virtual void add(const std::string& id, Persistent*) = 0;
	virtual void remove(const std::string& id) = 0;

	// only call this within Dump::dump
	virtual std::auto_ptr<res::res_out_stream> obj_store_stream() = 0;

	virtual ~StateMgr();
};


StateMgr& statemgr();


class StateHolder
{
	std::string id;
public:
	StateHolder(const std::string& i, Persistent* p)
	 : id(i)
	{
		statemgr().add(id, p);
	}
	~StateHolder()
	{
		statemgr().remove(id);
	}
};

void restore(const std::string& fn);

}
}
}

#endif

