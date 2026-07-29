
#ifndef REAPER_MAIN_TYPES_BASE_H
#define REAPER_MAIN_TYPES_BASE_H

#include <algorithm>

namespace reaper {



template<class Impl>
class Vec : public Impl
{
public:	
	typedef typename Impl::value_type value_type;

	Vec() : Impl{} { }
	Vec(const value_type* p) : Impl{} {
		std::copy_n(p, Impl::N, this->get());
	}
	Vec(value_type a, value_type b) : Impl{} {
		static_assert(Impl::N == 2, "two values require a two-dimensional vector");
		this->get()[0] = a;
		this->get()[1] = b;
	}
	Vec(value_type a, value_type b, value_type c) : Impl{} {
		static_assert(Impl::N == 3, "three values require a three-dimensional vector");
		this->get()[0] = a;
		this->get()[1] = b;
		this->get()[2] = c;
	}
	Vec(value_type a, value_type b, value_type c, value_type d) : Impl{} {
		static_assert(Impl::N == 4, "four values require a four-dimensional vector");
		this->get()[0] = a;
		this->get()[1] = b;
		this->get()[2] = c;
		this->get()[3] = d;
	}

	const value_type& operator[] (const int i) const { return this->get()[i]; }
	      value_type& operator[] (const int i)       { return this->get()[i]; }

};

struct Vec2 
{
	enum { N = 2 };
	float x, y;
	typedef float value_type;
};

struct Vec3
{
	enum { N = 3 };
	float x, y, z;
	typedef float value_type;
};

struct Vec4
{
	enum { N = 4 };
	float x, y, z, w;
	typedef float value_type;
};

template<class I>
struct GetX : public I
{
	typename I::value_type* get() { return &this->x; }
	const typename I::value_type* get() const { return &this->x; }
};


       
}

#endif

/*
* $Author: peter $
* $Date: 2002/01/23 04:42:50 $
* $Log: types_base.h,v $
* Revision 1.5  2002/01/23 04:42:50  peter
* mackes gfx-fix in igen, och fix av dylik... ;)
*
* Revision 1.4  2002/01/22 23:44:05  peter
* reversed last two revs
*
* Revision 1.2  2002/01/17 04:59:01  peter
* test
*
* Revision 1.1  2002/01/09 13:57:36  peter
* Yet Another Vector Implementation, this time SimpleYetFlexible(tm)  ;)
*
*
*/
