#include "hw/compat.h"

#include <string>
#include <map>

#include "main/exceptions.h"
#include "misc/unique.h"

namespace reaper {
namespace misc {
namespace {

typedef std::map<std::string, int> UniqueCont;
int count = 1;
UniqueCont values;

}

class unique_error : public fatal_error_base
{
	std::string error;
public:
	unique_error(const std::string &e) : error(e) {}
	const char* what() const { return error.c_str(); }
};

void Unique::reset() {
	count = 1;
	values.clear();
}

Unique::Unique(const std::string &s) : val(0)
{
	if(values.find(s) != values.end()) {
		val = values[s];
	} else {
		val = values[s] = count++;		
	}	
}

Unique::Unique(const char *str) : val(0)
{
	std::string s(str);

	if(values.find(s) != values.end()) {
		val = values[s];
	} else {
		val = values[s] = count++;		
	}	
}


std::string Unique::get_text() const {
	for(UniqueCont::const_iterator i = values.begin(); i != values.end(); ++i) {
		if(i->second == val) {
			return i->first;
		}
	}

	throw unique_error("FATAL: Value has no corresponding string representation!?!?");
}

}
}

