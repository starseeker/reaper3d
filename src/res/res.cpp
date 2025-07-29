

#include "hw/compat.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
// Modern C++ time handling headers - keep ctime for compatibility
#include <ctime>     // C time functions  
#include <chrono>    // Modern C++11 time handling
#include <iomanip>   // For std::put_time

#include "hw/debug.h"
#include "hw/osinfo.h"
#include "res/res.h"
#include "res/config.h"
#include "res/resource.h"
#include "res/zstream.h"
#include "misc/free.h"
#include "misc/parse.h"

#include "hw/stat.h"

namespace reaper
{
namespace res
{

using std::string;


bool is_dir(const string& path)
{
	stat_t st;
	return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

bool is_file(const string& path)
{
	stat_t st;
	return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

bool is_entry(const string& path)
{
	stat_t st;
	return stat(path.c_str(), &st) == 0
	    && (S_ISREG(st.st_mode) || S_ISDIR(st.st_mode));
}

string addslash(string p)
{
	int n = p.size();
	if (n > 0 && p[n-1] != '/')
		p += '/';
	return p;
}


string to_string(ResourceClass rc)
{
	switch(rc) {
	case Texture: 	 return "Texture";
	case Object:	 return "Object";
	case ObjectData: return "ObjectData";
	case Level:	 return "Level";
	case Terrain:	 return "Terrain";
	case File:	 return "File";
	case Sound:	 return "Sound";
	case Config:	 return "Config";
	case Music:	 return "Music";
	case Video:	 return "Video";
	case Playback:	 return "Playback";
	case Cache:	 return "Cache";
	case Screenshot: return "Screenshot";
	case Scenario:   return "Scenario"; 
	case Shader:     return "Shader"; 
	case GameState:  return "GameState";
	case AI:	 return "AI";
	default:
		throw resource_not_found("res::to_string() Unknown Resource Class!");
	}
}

string gen_name()
{
	// Modernized: Using C++17 std::chrono with improved error handling
	// Get current time using std::chrono::system_clock
	auto now = std::chrono::system_clock::now();
	auto time_t_now = std::chrono::system_clock::to_time_t(now);
	
	// Use modern C++ approach with safer time conversion
	std::tm local_tm = {};
	
	#ifdef WIN32
		// Windows has localtime_s which is thread-safe
		if (localtime_s(&local_tm, &time_t_now) == 0) {
			std::stringstream ss;
			ss << std::put_time(&local_tm, "save_%Y-%m-%d_%H-%M");
			return ss.str();
		}
	#else
		// Use localtime_r for thread-safe operation on POSIX systems
		if (localtime_r(&time_t_now, &local_tm) != nullptr) {
			std::stringstream ss;
			ss << std::put_time(&local_tm, "save_%Y-%m-%d_%H-%M");
			return ss.str();
		}
	#endif
	
	// Fallback: create a simple name if time conversion fails
	return "save_error";
}

string path_cat(string a, string b, string c = "")
{
	return addslash(a) + b + c;
}

struct res_file {
	string data, ident, ext;
	bool ex;

	res_file() : ex(false) { }
	res_file(string dd, string id, string ee = "", bool x = true)
	 : data(dd), ident(id), ext(ee), ex(x)
	{ }

	string to_str() {
		if (data.empty())
			return ident+ext;
		else
			return path_cat(data, ident, ext);
	}

	bool exists() const { return ex; }
};

string find_root_path()
{
	stat_t st;

	string dir("");
	for (int j = 0; j < 10; ++j) {
		string path = dir + "data/config";
		if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
			return dir;
		dir += "../";
	}
		
	return "./";
}

bool makedir(string dir)
{
	int r = ::mkdir(dir.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);
	return r == 0 || errno == EEXIST;
}

class Paths
{
	typedef std::deque<string> Stack;
	typedef Stack::iterator Iter;
	Stack datadirs;
	Stack plugindirs;
public:
	Paths()
	{
		reinit();
	}

	void reinit()
	{
		datadirs.clear();
		plugindirs.clear();
#ifdef DATADIR
		push_datadir(DATADIR);
#endif
#ifdef LIBDIR
		push_plugindir(LIBDIR);
#endif
		string root = find_root_path();
		push_datadir(path_cat(root, "data"));
		push_plugindir("plugins");
		push_plugindir(path_cat(root, "plugins"));

		const char* p = getenv("HOME");
		if (p && *p) {
			string home_rp = path_cat(p, ".reaper");
			if (is_dir(home_rp)) {
				push_datadir(home_rp);
			} else {
				if (makedir(home_rp))
					push_datadir(home_rp);
			}
		}
	}

	res_file find_data(string ident, string ext = "", bool only_top = false)
	{
		Iter c, e = (only_top ? (datadirs.begin()+1) : datadirs.end());
		for (c = datadirs.begin(); c != e; ++c) {
			if (!ext.empty() && is_entry(path_cat(*c, ident, ext)))
				return res_file(*c, ident, ext);
			if (is_entry(path_cat(*c, ident)))
				return res_file(*c, ident);
		}
		return res_file(datadirs.front(), ident, ext, false);
	}

	void push_datadir(string dir)
	{
		datadirs.push_front(addslash(dir));
	}

	string find_plugin(string ident, string alt_ext)
	{
		Iter c, e = plugindirs.end();
		for (c = plugindirs.begin(); c != e; ++c) {
			string t_def = path_cat(*c, ident, ".rp");
			if (is_file(t_def))
				return t_def;
			string t_alt = path_cat(*c, ident, alt_ext);
			if (is_file(t_alt))
				return t_alt;
		}
		throw resource_not_found(ident);
	}

	void push_plugindir(string d)
	{
		string dir = addslash(d);
		plugindirs.push_front(dir);
		plugindirs.push_front(dir + addslash(hw::os_name()));
	}
};

Paths& paths()
{
	static Paths inst;
	return inst;
}

void add_datapath(const std::string& dir)
{
	paths().push_datadir(dir);
}

void add_pluginpath(const std::string& dir)
{
	paths().push_plugindir(dir);
}

string find_plugin(const std::string& id, const std::string& alt_ext)
{
	return paths().find_plugin(id, alt_ext);
}

string flatten(string p)
{
	for (size_t i = 0; i < p.size(); ++i)
		p[i] = p[i] == '/' ? '_' : p[i];
	return p;
}

res_file res_resolver(ResourceClass rc, const string& id, bool force_homedir = false)
{
	res_file p;
	switch (rc) {
	case Texture: 
		p = res_file("textures", id, ".png");
		break;
	case Object:
		p = res_file("objects", id, ".ro");
		break;
	case ObjectData:
		p = res_file("objectdata", id, ".ro");
		break;
	case Level:
		p = res_file("levels", id, ".rl");
		break;
	case Terrain:
		p = res_file("terrains", id, ".rt");
		break;
	case File:
		p = res_file("", id, "");
		break;
	case Sound:
		p = res_file("sounds", id, ".wav");
		break;
	case Music:
		p = res_file("music", id, ".mp3");
		break;
	case Video:
		p = res_file("video", id, ".mpeg");
		break;
	case Config:
		p = res_file("config", id, "");
		break;
	case Playback:
		p = res_file("playback", id, "");
		break;
	case Cache:
		p = res_file("cache", flatten(id), "");
		break;
	case Screenshot:
		p = res_file("screenshots", id, ".png");
		break;
	case Scenario:
		p = res_file("scenario", id, "");
		break;
	case Shader:
		p = res_file("shaders", id, ".rs");
		break;
	case GameState:
		p = res_file("gamestate", id, ".rd");
		break;
	case AI:
		p = res_file("ai", id, ".ng");
		break;
	case Plugin:
		throw resource_not_found(id + " - do not use res_resolver for plugins");
	default:
		throw resource_not_found(id);
	}
	
	return paths().find_data(path_cat(p.data, p.ident), p.ext, force_homedir);
}

res_stream::ResID::ResID(ResourceClass r, const string& i, res_flags flg)
 : rc(r), id(i), flags(flg)
{ }

void res_stream::close()
{
	misc::zero_delete(sb);
	misc::zero_delete(sb2);
}

res_stream::res_stream(ResourceClass rc, const string& id, res_flags flg)
 : std_istream(0), sb(0), sb2(0), res(rc, id, flg)
{
	res_init(res);
}

res_stream::res_stream(const ResID& rc)
 : std_istream(0), sb(0), sb2(0), res(rc)
{
	res_init(res);
}

void res_stream::res_init(const ResID& r)
{
	res_file path = res_resolver(r.rc, r.id);

	std::ios::openmode flags = std::ios::in | std::ios::binary;

	std::filebuf *fb = new std::filebuf;
	if (fb->open(path.to_str().c_str(), flags)) {
		sb = fb;
		init(sb);
		return;
	} else {
		delete fb;
		if (r.flags & throw_on_error)
			throw resource_not_found(path.to_str());
	}
}

res_stream::~res_stream()
{
	misc::zero_delete(sb);
	misc::zero_delete(sb2);
}

const string res_stream::id() const {
	return res.id;
}

res_stream::ResID res_stream::clone() const
{
	return res;
}

bool try_mkdir(string h, string p)
{
	int i = p.find('/');
	while (i < p.size()) {
		if (!makedir((h + p.substr(0, i))))
			return false;
		i = p.find('/', i+1);
	}
	return true;
}


res_out_stream::res_out_stream(ResourceClass rc, const string& id, bool text_mode)
 : std_ostream(0), sb(0)
{
	res_file fp = res_resolver(rc, id, true);
	string fn = fp.to_str();

	std::filebuf* fb = new std::filebuf;
	std::ios::openmode flags =  std::ios::out | std::ios::trunc;

	if (! text_mode)
		flags |= std::ios::binary;
	if (fb->open(fn.c_str(), flags)) {
		init(fb);
		sb = fb;
		return;
	}
	string base = fp.data;
	if (try_mkdir(base, fn.substr(base.size())) &&
	    fb->open(fn.c_str(), flags)) {
		init(fb);
		sb = fb;
		return;
	}
	delete fb;
}

res_out_stream::~res_out_stream()
{
	delete sb;
}

resource_not_found::resource_not_found(const string& s)
{
	msg = string("Resource: ") + s + " not found";
}

const char* resource_not_found::what() const
{
	return msg.c_str();
}

bool is_newer(ResourceClass rc, const string& id1, const string& id2)
{
	return is_newer(rc, id1, rc, id2);
}

bool is_newer(ResourceClass rc1, const string& id1,
	      ResourceClass rc2, const string& id2)
{
	res_file rf1 = res_resolver(rc1, id1);
	res_file rf2 = res_resolver(rc2, id2);
	stat_t st1, st2;
	stat(rf1.to_str().c_str(), &st1);
	stat(rf2.to_str().c_str(), &st2);

	return st1.st_mtime > st2.st_mtime;
}

namespace {
	debug::DebugOutput derr("check");
}


bool check(char type, const string& path)
{
	string rpath = paths().find_data(path).to_str();
	switch (type) {
	case 'f':
		if (is_file(rpath))
			break;
		derr << "File not found: " << rpath << '\n';
		return false;
	case 'd':
		if (is_dir(rpath))
			break;
		derr << "Directory not found: " << rpath << '\n';
		return false;
	}
	return true;
}


bool sanity_check()
{
	res_file dist = paths().find_data("config/dist");
	if (! dist.exists()) {
		derr << "cannot find <reaper-data>/config/dist\n";
		return false;
	}

	try {
		bool ok = true;
		ConfigEnv chk("dist");
		ConfigEnv::Env::const_iterator c, e = chk.end();
		for (c = chk.begin(); c != e; ++c) {
			misc::stringpair p = *c;
			if (p.first.empty() || p.second.empty()) {
				derr << "Malformed line in dist file: "
					<< p.first << ' ' << p.second << '\n';
				ok = false;
				continue;
			}
			ok = ok && check(p.second[0], p.first);
		}
		return ok;
	} catch (resource_not_found) {
		return false;
	}
}



}
}

