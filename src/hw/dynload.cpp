
#include "hw/compat.h"

#include "hw/dynload.h"
#include "hw/debug.h"
#include "hw/exceptions.h"
#include "hw/osinfo.h"
#include "res/res.h"
#include "hw/stat.h"

#ifdef WIN32

#include "hw/windows.h"
#include "hw/win32_errors.h"

#else

#include <dlfcn.h>

#endif

namespace reaper {
namespace hw {
namespace dynload {

namespace { debug::DebugOutput derr("hw::dynload", 5); }

#ifdef WIN32

const int RTLD_GLOBAL = 0;
const int RTLD_LAZY = 0;


const std::string dlerror()
{
	return win32_strerror();
}

HINSTANCE dlopen(const char* fn, int flags)
{
	return LoadLibrary(fn);
}

symptr dlsym(HINSTANCE h, const char* sym)
{
	return (symptr)GetProcAddress(h, sym);
}

int dlclose(HINSTANCE h)
{
	return ! FreeLibrary(h);
}

const std::string dl_ext = ".dll";

#else

const std::string dl_ext = ".so";

#endif

void error(const std::string& a, const std::string& b)
{
	throw hw_error(a + ": " + b + ' ' + dlerror());
}


DynLoader::DynLoader(const std::string& name)
 : dlname(name), handle(0)
{
	derr << "loading: " << name << "\n";
	std::string path = res::find_plugin(dlname, dl_ext);
	if (!(handle = dlopen(path.c_str(), RTLD_GLOBAL | RTLD_LAZY)))
		error("Error loading", name);
}

DynLoader::~DynLoader()
{
	derr << "closing " << dlname << '\n';
	if (dlclose(handle) != 0) {
		error("Error unloading", dlname);
	}
}

symptr DynLoader::load(const std::string& symbol)
{
	symptr s = (symptr)dlsym(handle, symbol.c_str());
	if (s == 0)
		error("Error locating symbol", symbol);
	return s;
}

}
}
}

