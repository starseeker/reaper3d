

#include "hw/compat.h"

#include <string>
#include <sstream>

#include "res/resource.h"
#include "res/config.h"
#include "res/res.h"
#include "hw/debug.h"
#include "misc/sequence.h"
#include "misc/parse.h"
#include "main/types_io.h"

namespace reaper
{
namespace res
{

using std::string;
using namespace misc;

namespace {
	debug::DebugOutput dlog("config", 5);
	const std::string empty_str;
}


bool ConfVal::operator==(bool b) const
{
	return (b == bool(*this));
}

bool ConfVal::operator==(int i) const
{
	return i == int(*this);
}

bool ConfVal::operator==(long l) const
{
	return l == long(*this);
}

bool ConfVal::operator==(float f) const
{
	return f == float(*this);
}

bool ConfVal::operator==(const string& s) const
{
	return s == string(*this);
}

bool ConfVal::operator==(const char* p) const
{
	return string(p) == string(*this);
}

ConfVal::operator bool() const
{
	return (val == "1") || (strequal(val.substr(0, 4), "true"));
}


ConfVal::operator int() const
{
	if (val.empty())
		return 0;
	std::istringstream istr(val);
	int ival = 0;
	istr >> ival;
	return ival;
}

ConfVal::operator long() const
{
	if (val.empty())
		return 0;
	std::istringstream istr(val);
	long lval = 0;
	istr >> lval;
	return lval;
}


ConfVal::operator float() const
{
	if (val.empty())
		return 0;
	std::istringstream fstr(val);
	float fval = 0;
	fstr >> fval;
	return fval;
}

ConfVal::operator const string() const
{
	return val;
}

ConfVal::operator Point() const
{
	return read<Point>(val);
}

ConfVal::operator Vector() const
{
	return read<Vector>(val);
}

ConfVal::operator Matrix() const
{
	return read<Matrix>(val);
}

MConfVal& MConfVal::operator=(bool b)
{
	mval = (b) ? "true" : "false";
	return *this;
}

MConfVal& MConfVal::operator=(int i)
{
	mval = ltos(i);
	return *this;
}

MConfVal& MConfVal::operator=(long l)
{
	mval = ltos(l);
	return *this;
}

MConfVal& MConfVal::operator=(float f)
{
	mval = ftos(f);
	return *this;
}

MConfVal& MConfVal::operator=(const string& s)
{
	mval = s;
	return *this;
}


class ConfigNode : public NodeConfig<ConfigEnv> {
	Ident pre, ext;

	Ident mk_path(IdentRef id) {
		if (id.find('/') == string::npos)
			return "config/" + pre + id + ext;
		else
			return pre + id + ext;
	}
	void read_env(ConfigEnv& env, IdentRef id) {
		res_stream fr(File, mk_path(id));
		if (fr.good()) {
			env.read(fr);
		}
	}
public:
	typedef tp<ConfigEnv>::ptr Ptr;

	ConfigNode(IdentRef p, IdentRef x)
	 : pre(p), ext(x)
	{ }

	Ptr create(IdentRef id) {
		Ptr p = Ptr(new ConfigEnv());
		Ptr p2(0);
		if (parent() && (p2 = parent()->create(id)) != 0)
			p->merge(*p2);

		read_env(*p, id);
		dlog << "Reading config: " << pre << id << ext << '\n';
		return p;
	}

	void save(IdentRef id, const ConfigEnv& env) {
		res_out_stream out(File, mk_path(id), true);
		out << env;
	}
};

struct InitLoader {
	InitLoader() {
		push_config<ConfigEnv>(new ConfigNode("", ".defaults"), "configenv");
		push_config<ConfigEnv>(new ConfigNode("", ""));
	}
} stat_init;

ConfigEnv::ConfigEnv() { }

ConfigEnv::ConfigEnv(const string& m)
 : module(m)
{
	merge(resource<ConfigEnv>(m));
}

ConfigEnv::ConfigEnv(const ConfigEnv& env)
{
	merge(env);
}

ConfigEnv::ConfigEnv(std::istream& is, bool stop_at_empty_line)
{
	config_lines(is, env, stop_at_empty_line);
}

ConfigEnv& ConfigEnv::operator=(const ConfigEnv& o)
{
	merge(o);
	return *this;
}

void ConfigEnv::read(std::istream& is, bool stop_at_empty_line)
{
	config_lines(is, env, stop_at_empty_line);
}

bool ConfigEnv::defined(const string& var) const
{
	return env.find(var) != env.end();
}


const ConfVal ConfigEnv::operator[](const string& var) const
{
	Env::const_iterator i = env.find(var);
	return (i != env.end()) ? ConfVal(i->second) : ConfVal(empty_str);
}

MConfVal ConfigEnv::operator[](const string& var)
{
	return MConfVal(env[var]);
}

struct write {
	std::ostream& os;
	write(std::ostream& o) : os(o) { }
	int operator()(std::pair<const string, string>& p) const {
		os << p.first << ": " << p.second << '\n';
		return 42;
	}
};

void ConfigEnv::merge(const ConfigEnv& other)
{
	for (Env::const_iterator i = other.begin(); i != other.end(); ++i) {
		env.insert(*i);
	}
}

std::ostream& operator<<(std::ostream& os, const ConfigEnv& env)
{
	ConfigEnv::iterator c = env.begin(), e = env.end();
	bool in_subsec = false;

	while (c != e) {
		string pfx = c->first.substr(0, c->first.find('_'));
		ConfigEnv::iterator n = c;
		while (n != e && pfx == n->first.substr(0, pfx.size()))
			++n;
		if (std::distance(c, n) > 3) {
			os << "\n[" << pfx << "]\n";
			for (; c != n; ++c)
				os << c->first.substr(pfx.size()+1) << ": " << c->second << '\n';
			os << '\n';
			in_subsec = true;
		} else {
			if (in_subsec)
				os << "\n[]\n";
			for (; c != n; ++c)
				os << c->first << ": " << c->second << '\n';
		}
	}
	return os;
}

}

}


