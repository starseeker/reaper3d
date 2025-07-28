#ifndef WORLD_SEARCH_H
#define WORLD_SEARCH_H

#include <deque>

#include "misc/free.h"
#include "world/query_obj.h"

namespace reaper
{
namespace world
{

const float max_hash_val = 1e20;

template<class T>
float calc_key(T t) {
	if (t->is_dead())
		return max_hash_val;
	else
		return t->get_pos().x + t->get_pos().z;
}


template<class T>
struct Box {
	T o;
	mutable float key;
	Box(T obj) : o(obj), key(calc_key(obj)) { }
	Box(float k) : key(k) { }
	T operator->() const { return o; }

	operator T() const { return o; }

	bool operator<(const Box<T>& b) const {
		return key < b.key;
	}
	bool operator==(const T& t) const { return o == t; }

	T get() const { return o; }
};



template<class T>
struct SearchCont : public std::deque<Box<T> > {
	typedef typename std::deque<Box<T> >::iterator iterator;
	typedef T value_type;
};


typedef std::pair<float,float> ftuple;

template<class Cont>
class search_iterator
{

	typedef typename Cont::iterator EL_it;

	enum What { F_All, F_Cylinder, F_Sphere, F_Frustum, F_Line, F_Rect } what;
	Sphere sphere;
	Frustum frustum;
	Rect rect;
	Cont* cont;
	EL_it cur, end;

	void find();
	void init();
	void init_find();
	bool test_next();
//	bool find_first();
	ftuple get_limits();
public:
	typedef std::forward_iterator_tag iterator_category;
	typedef typename Cont::value_type real_value_type;
	typedef Box<real_value_type> value_type;
	typedef int difference_type;
	typedef value_type pointer;
	typedef value_type& reference;
	typedef int distance_type;

	/*
	search_iterator(const search_iterator& i) {
		*this = i;
	}
	*/
	/*
	search_iterator& operator=(const search_iterator& i) {
		what = i.what;
		sphere = i.sphere;
		frustum = i.frustum;
		cont = i.cont;
		cur = i.cur;
		end = i.end;
		return *this;
	}
*/
	search_iterator() : cont(0) { }

	search_iterator(Cont* c, bool all)
	 : what(F_All), cont(c)
	{
		if (all)
			init();
		else
			init_find();
	}

	search_iterator(Cont* ct, const Cylinder& c)
	 : what(F_Cylinder), sphere(Point(c.p.x, 0, c.p.y), c.r),
	   cont(ct)
	{
		init_find();
	}

	search_iterator(Cont* c, const Sphere& s)
	 : what(F_Sphere), sphere(s), cont(c)
	{
		init_find();
	}

	search_iterator(Cont* c, const Frustum& f)
	 : what(F_Frustum), frustum(f), cont(c)
	{
		init_find();
	}

	search_iterator(Cont* c, const Line& l)
	 : what(F_Line), frustum(l.p1, l.p2.to_vec(), Vector(), 0, 0),
	   cont(c)
	{
		init_find();
	}

	search_iterator(Cont* c, const Rect& r)
	 : what(F_Rect), rect(r), cont(c)
	{
		init_find();
	}

	~search_iterator() { }

	reference operator*() {
		return *cur;
	}

	const value_type& operator*() const {
		return *cur;
	}

	pointer operator->() {
		return *cur;
	}

	const pointer operator->() const {
		return *cur;
	}

	search_iterator& operator++() {
		++cur;
		find();
		return *this;
	}


	search_iterator operator++(int) {
		search_iterator<Cont> i = *this;
		++(*this);
		return i;
	}


	bool operator!=(const search_iterator<Cont>& i) const {
		return !(*this == i);
	}

	bool operator==(const search_iterator<Cont>& i) const {
		if (cont && i.cont)
			return cur == i.cur;
		else
			return cont == i.cont;
	}

	void erase();
};

}
}
#endif

/*
 * $Author: pstrand $
 * $Date: 2002/04/06 20:10:07 $
 * $Log: search.h,v $
 * Revision 1.22  2002/04/06 20:10:07  pstrand
 * quadtree dynamicification ;)
 *
 * Revision 1.21  2002/03/18 13:34:08  pstrand
 * rect search & level fix
 *
 * Revision 1.20  2002/01/31 04:54:13  peter
 * no message
 *
 * Revision 1.19  2002/01/24 20:57:49  peter
 * upprensning
 *
 * Revision 1.18  2001/11/29 00:53:59  peter
 * 'erase'
 *
 * Revision 1.17  2001/11/27 00:54:55  peter
 * worlditeratorer lämnar inte ifrån sig döda objekt längre..
 *
 * Revision 1.16  2001/11/10 13:57:05  peter
 * minnesfixar...
 *
 * Revision 1.15  2001/11/10 11:58:37  peter
 * diverse minnesfixar och strul...
 *
 * Revision 1.14  2001/08/20 20:31:43  peter
 * no message
 *
 * Revision 1.13  2001/08/20 17:11:45  peter
 * obj.ptr
 *
 * Revision 1.12  2001/08/06 12:16:46  peter
 * MegaMerge (se strandy_test-grenen för diffar...)
 *
 * Revision 1.11.4.3  2001/08/03 13:44:17  peter
 * pragma once obsolete...
 *
 * Revision 1.11.4.2  2001/08/02 16:29:14  peter
 * objfix...
 *
 * Revision 1.11.4.1  2001/07/31 17:34:10  peter
 * testgren...
 *
 * Revision 1.11  2001/07/11 10:56:37  peter
 * uppdateringar (strul med gcc3 och optimeringar!)
 *
 * Revision 1.10  2001/07/06 01:47:39  macke
 * Refptrfix/headerfilsstäd/objekt-skapande/mm
 *
 * Revision 1.9  2001/05/12 22:06:09  peter
 * *** empty log message ***
 *
 * Revision 1.8  2001/05/12 19:57:56  peter
 * no message
 *
 */



