
#include "hw/compat.h"

#include <map>

#include "hw/debug.h"
#include "misc/sequence.h"
#include "misc/iostream_helper.h"
#include "main/exceptions.h"
#include "game/state.h"
#include "res/config.h"
#include "res/resource.h"
#include "object/objectgroup.h"
#include "misc/free.h"
#include "res/res.h"

#include <deque>
#include <iostream>

namespace reaper {
namespace game {
namespace state {

namespace {
	reaper::debug::DebugOutput derr("game::state");
}

using namespace std;
using namespace misc;

namespace foo { debug::DebugOutput derr("statemgr"); }

define_reaper_exception(statemgr_err);

struct dump_it
{
	ostream& os;
	dump_it(ostream& o) : os(o) { }
	void operator()(const pair<string, Persistent*>& p)
	{
		res::ConfigEnv env;
		p.second->dump(env);
		foo::derr << "dumping: " << p.first << '\n';
		os << "Section: " << p.first << '\n';
		os << env;
		os << '\n';
	}
};

StateMgr::~StateMgr()
{ }

class StateMgrImpl
 : public StateMgr
{
	string fn;
	map<string, Persistent*> pmap;
public:
	StateMgrImpl() { }

	void dump(const string& fn) const
	{
		res::res_out_stream os(res::GameState, fn);
		for_each(cseq(pmap), dump_it(os));
	}
/*
	void restore(std::istream& is)
	{
		foo::derr << "restore\n";
		while (!is.eof()) {
			std::string junk, sec;
			misc::config_line(is, junk, sec);
			if (junk != "Section")
				throw statemgr_err("dumpfile format error, no section: " + junk);
			foo::derr << "restore: " << sec << '\n';
			pmap[sec]->restore(is);
		}
	}
*/
	void add(const string& id, Persistent* p)
	{
		pmap[id] = p;
	}

	void remove(const string& id)
	{
		pmap.erase(id);
	}

	std::auto_ptr<res::res_out_stream> obj_store_stream() {
		return std::auto_ptr<res::res_out_stream>(
			new res::res_out_stream(res::GameState, fn + "_objs"));
	}
};

StateMgr& statemgr()
{
	static StateMgrImpl s;
	return s;
}

using namespace res;

typedef map<string, ConfigEnv> ConfigMap;
typedef map<string, object::ObjectGroup> ObjGrpMap;

struct SavedGame
{
	ConfigMap cfg;
	ObjGrpMap grp;
};

class GameLoader : public NodeConfig<SavedGame>
{
public:
	Ptr create(IdentRef id)
	{
		SavedGame* g = new SavedGame();
		res::res_stream is(res::GameState, id);
		while (! is.eof()) {
			string junk, sec;
			config_line(is, junk, sec);
			while (!is.eof() && !elem(is.peek(), "\r\n")) {
				string var, val;
				config_line(is, var, val);
				g->cfg[sec][var] = val;
			}
			is >> crlf;			
		}
		res::res_stream isg(res::GameState, id + "_objs");
		while (! isg.eof()) {
			string junk, lbl;
			config_line(isg, junk, lbl);
			object::ObjectGroup og;

			while (!isg.eof() && !elem(isg.peek(), "\r\n")) {
				object::MkInfo mk;
				isg >> mk;
				og.objs.push_back(mk);
			}
			g->grp[lbl] = og;
			isg >> crlf;
		}
		return g;
	}
};

typedef SmartPtr<SavedGame> SPtr;

class OverrideFilter : public NodeConfig<ConfigEnv>
{
	string me;
	SPtr game;
public:
	OverrideFilter(const string& m, SPtr p) : me(m), game(p) { }
	Ptr create(IdentRef id) {
		ConfigMap::iterator i = game->cfg.find(id);
		if (i != game->cfg.end())
			return new ConfigEnv(game->cfg[id]);
		else
			return Ptr(0);
	}
};

class OverrideObjGrp : public NodeConfig<object::ObjectGroup>
{
	SPtr game;
public:
	OverrideObjGrp(SPtr p) : game(p) { }
	Ptr create(IdentRef id) {
		return Ptr(new object::ObjectGroup(game->grp[id]));
	}
};

void restore(const string& fn)
{
	push_config<SavedGame>(new GameLoader(), "savegame");

	SmartPtr<SavedGame> s = resource_ptr<SavedGame>("latest");

	push_config<ConfigEnv>(new OverrideFilter("user", s));

	push_config<object::ObjectGroup>(new OverrideObjGrp(s));
}




}
}
}

