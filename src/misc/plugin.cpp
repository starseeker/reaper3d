
#include "hw/compat.h"

#include "hw/debug.h"
#include "hw/dynload.h"

#include "misc/sequence.h"
#include "misc/free.h"
#include "misc/plugin.h"
#include "world/world.h"

#include <stdio.h>

namespace reaper {
namespace misc {


using std::string;

int del_crts(const std::pair<std::string, PluginBase::Creator>& p) {
	delete p.second.dyn;
	return 0;
}

PluginBase::PluginBase()
{
}

PluginBase::~PluginBase()
{
	erase();
}

void* PluginBase::create(const string& name, void* arg, Cache)
{
	Plugins::iterator i(plugins.find(name));
	if (i == plugins.end())
		return (plugins[name] = create(name, arg));
	else
		return i->second;
}

void* PluginBase::create(const string& name, void* arg, NoCache)
{
	Creators::iterator i(creators.find(name));
	VoidCreator1 fn;
	if (i != creators.end()) {
		fn = i->second.creator;
	}
	else {
		Creator crt;
		crt.dyn = new hw::dynload::DynLoader(name);
		crt.creator = crt.dyn->load(string("create_") + name);
		creators[name] = crt;
		fn = crt.creator;
	}
	return fn(arg);
}

void PluginBase::erase()
{
	for_each(seq(creators), del_crts);
}

}
}



