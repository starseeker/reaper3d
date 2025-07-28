
/* $Id: mapping.h,v 1.16 2002/01/17 04:58:44 peter Exp $ */

#ifndef REAPER_HW_MAPPING_H
#define REAPER_HW_MAPPING_H

#include <string>

namespace reaper {
namespace hw {
namespace event {


class id_map
: public Mapping
{
public:
	virtual Event operator()(const Event& ev)
	{
		return ev;
	}
};



template<class Func>
class Map
 : public Mapping
{
	Func func;
public:
	Map(const Func& f) : func(f) { }
	virtual Event operator()(const Event& ev)
	{
		return func(ev);
	}
};


/// Create (internal) inputmapping from customized map.
template<class Func>
Mapping* create_mapping(Func map)
{
	return new Map<Func>(map);
}

Mapping* create_game_map(const std::string& name);

}
}
}

#endif

