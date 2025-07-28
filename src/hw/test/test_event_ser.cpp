
#include "hw/compat.h"

#include <sstream>

#include "hw/event.h"
#include "hw/debug.h"

using namespace reaper::hw::event;

reaper::debug::DebugOutput derr;


int main()
{
	std::ostringstream os;
	Event ek(id::Enter, true), ea(0, 0.314f);
	os << ek << ' ' << ea;
	derr << "|" << ek << "|  |" << ea << "|\n";
	std::istringstream is(os.str());
	Event ek2, ea2;
	is >> ek2 >> ea2;
	derr << "|" << ek2 << "|  |" << ea2 << "|\n";
	return (ek != ek2 || ea != ea2);
}


