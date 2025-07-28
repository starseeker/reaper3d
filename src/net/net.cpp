
#include "hw/compat.h"

#include <iostream>

#include "main/types_io.h"
#include "net/net.h"
#include "misc/iostream_helper.h"

namespace reaper {
namespace net {

using misc::spaces;

std::ostream& operator<<(std::ostream& os, const ObjState& st)
{
	os << st.id << ' ' << std::hex << st.sync << ' '
	   << st.mat << ' ' << st.vel << ' ' << st.rot_vel << ' '
	   << st.dead;
	return os;
}

std::istream& operator>>(std::istream& is, ObjState& st)
{
	is >> st.id >> spaces >> std::hex >> st.sync >> spaces
	   >> st.mat >> spaces >> st.vel >> spaces >> st.rot_vel >> spaces
	   >> st.dead;
	return is;
}



}
}

