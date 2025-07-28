
#include "hw/compat.h"
#include "hw/osinfo.h"

namespace reaper {
namespace hw {

std::string os_name()
{
#ifdef WIN32
	return "win32";
#else
# ifdef LINUX
	return "linux";
# else
#  ifdef SOLARIS
	return "solaris";
#  else
#   ifdef CYGWIN
	return "win32";
#   else
#    ifdef FREEBSD
	return "freebsd";
#    else
#     error Unknown operating system
#    endif
#   endif
#  endif
# endif
#endif
}

}
}
