


#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <system_error>

#include "hw/abstime.h"
#include "hw/debug.h"
#include "res/res.h"
#include "res/config.h"
#include "res/resource.h"
#include "res/zstream.h"
#include "misc/free.h"
#include "misc/parse.h"

namespace reaper
{
namespace res
{

using std::string;
namespace fs = std::filesystem;


bool is_dir(const string& path)
{
	std::error_code error;
	return fs::is_directory(path, error);
}

bool is_file(const string& path)
{
	std::error_code error;
	return fs::is_regular_file(path, error);
}

bool is_entry(const string& path)
{
	std::error_code error;
	const fs::file_status status = fs::status(path, error);
	return !error &&
	       (fs::is_regular_file(status) || fs::is_directory(status));
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
	return hw::time::strtime("save_%Y-%m-%d_%H-%M");
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
	string dir("");
	for (int j = 0; j < 10; ++j) {
		string path = dir + "data/config";
		if (is_dir(path))
			return dir;
		dir += "../";
	}
		
	return "./";
}

bool makedir(string dir)
{
	std::error_code error;
	return fs::create_directories(dir, error) ||
	       (!error && fs::is_directory(dir));
}

class Paths
{
	typedef std::deque<string> Stack;
	typedef Stack::iterator Iter;
	Stack datadirs;
public:
	Paths()
	{
		reinit();
	}

	void reinit()
	{
		datadirs.clear();
#ifdef DATADIR
		push_datadir(DATADIR);
#endif
		string root = find_root_path();
		push_datadir(path_cat(root, "data"));

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
	rdbuf(nullptr);
	buffer.reset();
}

res_stream::res_stream(ResourceClass rc, const string& id, res_flags flg)
 : std_istream(nullptr), res(rc, id, flg)
{
	res_init(res);
}

res_stream::res_stream(const ResID& rc)
 : std_istream(nullptr), res(rc)
{
	res_init(res);
}

void res_stream::res_init(const ResID& r)
{
	res_file path = res_resolver(r.rc, r.id);

	std::ios::openmode flags = std::ios::in | std::ios::binary;

	auto file = std::make_unique<std::filebuf>();
	if (file->open(path.to_str().c_str(), flags)) {
		init(file.get());
		buffer = std::move(file);
		return;
	}
	if (r.flags & throw_on_error)
		throw resource_not_found(path.to_str());
}

const string res_stream::id() const {
	return res.id;
}

res_stream::ResID res_stream::clone() const
{
	return res;
}

res_out_stream::res_out_stream(ResourceClass rc, const string& id, bool text_mode)
 : std_ostream(nullptr)
{
	res_file fp = res_resolver(rc, id, true);
	string fn = fp.to_str();

	auto file = std::make_unique<std::filebuf>();
	std::ios::openmode flags =  std::ios::out | std::ios::trunc;

	if (! text_mode)
		flags |= std::ios::binary;
	if (file->open(fn.c_str(), flags)) {
		init(file.get());
		buffer = std::move(file);
		return;
	}
	std::error_code error;
	const fs::path parent = fs::path(fn).parent_path();
	if (!parent.empty())
		fs::create_directories(parent, error);

	if (!error && file->open(fn.c_str(), flags)) {
		init(file.get());
		buffer = std::move(file);
		return;
	}
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
	std::error_code first_error;
	std::error_code second_error;
	const auto first_time = fs::last_write_time(rf1.to_str(), first_error);
	const auto second_time = fs::last_write_time(rf2.to_str(), second_error);

	return !first_error && !second_error && first_time > second_time;
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
	} catch (const resource_not_found&) {
		return false;
	}
}



}
}
