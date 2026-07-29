
#ifndef REAPER_MISC_FREE_H
#define REAPER_MISC_FREE_H

#include <vector>

namespace reaper {
namespace misc {

struct delete_it_t {
	template<class T, class A>
	void operator()(const std::pair<A, T>& p) const {
		delete p.second;
	}

	template<class T>
	void operator()(T* t) const {
		delete t;
	}
};

inline constexpr delete_it_t delete_it{};

template<class T>
inline void zero_delete(T& t)
{
	if (t) {
		delete t;
		t = nullptr;
	}
}

}
}

#endif
