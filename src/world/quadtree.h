
#ifndef REAPER_WORLD_QUADTREE_H
#define REAPER_WORLD_QUADTREE_H

#include "hw/compat.h"
#include "hw/debug.h"

#include <list>
#include <map>
#include <stack>
#include <deque>
#include <iostream>
#include <algorithm>

#include "misc/free.h"
#include "world/query_obj.h"
#include "world/geometry.h"
#include "world/triangle.h"

namespace reaper
{
namespace quadtree
{

typedef int Quadrant;

/** A square.
 *  With the lower-left corner located at (xoff, yoff),
 *  and a side length of 2 * half_size.
 */

class Square
{
public:
	float xoff, yoff, half_size;

	Square() : xoff(0), yoff(0), half_size(0) { }

	Square(float x, float y, float sz)
	 : xoff(x), yoff(y), half_size(sz)
	{ }

	/** Return a shrinked square, towards the given quadrant. */
	Square shrinked(Quadrant q) const;
	/** Returns a (8x) grown square */
	Square grown() const;
};

/*
std::ostream& operator<<(ostream& os, const Square& sq)
{
	os << sq.xoff << '\t' << sq.yoff << '\t' << sq.half_size;
	return os;
}
*/


// Helpers for quad-tree - these are just approximations...

/*
 convensions used in this file:
  (Point, Point) denotes a 3d-box
  (Point2D, Point2D) denotes a rectangle in the xz-plane
  (Point, float) denotes a sphere
  (Point2D, float) denotes a circle in the xz-plane (or an infinite cylinder)

 this will be made type-safe at some point...
*/




/** Calculate which subsquare a given rectangle occupies.
 *  \param ll lower-left corner
 *  \param ur upper-right corner
 *  \param sq square
 *  \return The occupied quadrant, or Ext if any boundary is crossed.
 */

Quadrant get_quadrant(const Point2D& ll, const Point2D& ur, const Square& sq);

Quadrant get_quadrant(const Point& p1, const Point& p2, const Square& sq);
Quadrant get_quadrant(Point2D c, float r, Square sq);
Quadrant get_quadrant(Point c, float r, Square sq);




/* The quadtree is parameterized by a class containing
 * all element-type specific calculations
 */


/* A box for elements, buffering common computations.  */
template<class Elem>
struct Box
{
	world::Sphere sp;
	Elem elem;
	int count;
	bool deleted;
	int visitors[4];
private:
	Box(const Box& b);
public:
	Box(float x, float y) {
		sp.p.x = x;
		sp.p.z = y;
	}

	Box(const world::Sphere& s)
	 : sp(s), count(0), deleted(true)
	{ }

	Box(const world::Sphere& s, Elem e)
	 : sp(s), elem(e), count(0), deleted(false)
	{ visitors[0] = visitors[1] = visitors[2] = visitors[3] = 0; }

	void update(const world::Sphere& s)
	{
		sp = s;
	}

	void clear()
	{
		deleted = true;
		elem = 0;
		count = 0;
	}

	Box* spawn() {
		Box* b = new Box(sp, elem);
		clear();
		return b;
	}

	const Elem& unbox() const { return elem; }
	Elem& unbox() { return elem; }
	bool operator<(const Box& o) const {
		return (sp.p.x+sp.p.z) < (o.sp.p.x+o.sp.p.z);
	}
	const Point& pos() const { return sp.p; }
	float radius() const { return sp.r; }

	float get_height() const { return sp.p.y+sp.r; }

	bool operator==(const Elem& e) const { return elem == e; }
	bool operator!=(const Elem& e) const { return elem != e; }

};

struct BoxComp {

	template<class T>
	int operator()(const Box<T>* b1, const Box<T>* b2) const {
		return *b1 < *b2;
	}
	
};

template<class T>
struct Vector {
	typedef std::vector<Box<T>*> t;
	typedef typename t::iterator iterator;

	static iterator lower_bound(t& s, const Box<T>* v) {
		return s.begin();
//		return std::lower_bound(s.begin(), s.end(), v, BoxComp());
	}
	static iterator upper_bound(t& s, const Box<T>* v) {
		return s.end();
//		return std::upper_bound(s.begin(), s.end(), v, BoxComp());
	}
	static void insert(t& s, Box<T>* c) {
		s.push_back(c);
	}

	static iterator begin(t& s) { return s.begin(); }
	static iterator end(t& s)   { return s.end(); }

	static void sort(t& s) {
	//	qt_sort(s.begin(), s.end());
	}
};


template<class T>
struct Deque {
	typedef std::deque<Box<T>*> t;
	typedef typename t::iterator iterator;

	static iterator lower_bound(t& s, const Box<T>& v) {
		return std::lower_bound(s.begin(), s.end(), v);
	}
	static iterator upper_bound(t& s, const Box<T>& v) {
		return std::upper_bound(s.begin(), s.end(), v);
	}
	static void insert(t& s, const Box<T>* c) {
		s.push_back(c);
		std::sort(s.begin(), s.end());
	}
};

template<class T>
struct MultiSet {
	typedef std::multiset<Box<T>*> t;
	typedef typename t::iterator iterator;

	static iterator lower_bound(t& s, const Box<T>& v) {
		return s.lower_bound(v);
	}
	static iterator upper_bound(t& s, const Box<T>& v) {
		return s.upper_bound(v);
	}
	static void insert(t& s, const Box<T>* v) {
		s.insert(v);
	}
};

template<class T>
struct NodeCont : public Vector<T>
{
};


template<class E>
class QuadTree;
template<class E>
class QuadTree_dfs_iterator;

template<class E>
class Node
{
	friend class QuadTree<E>;
	friend class QuadTree_dfs_iterator<E>;

	enum NodeType { IntNode, TmpLeaf, Leaf };
	NodeType state;
	Square geom;

	Node* quad[4];
	typename NodeCont<E>::t elems;

	float max_size;
	float max_height;
	int depth;

public:
	typedef NodeCont<E> NS;
	typedef typename NodeCont<E>::iterator iterator;

	Node(const Square& sq, int depth);
	~Node();
	void insert(Box<E>*);

	bool empty() const;

	void update(Node<E>*);
//	void remove(const Quadtree_dfs_iterator<E>& i);

	void show(int level);
};


/** Decision-helper for square-calculations.
 *  Calculates the subsquare for a given query-object
 *  and the current square.
 *  And decides if an element is acceptable for a given query.
 */

template<class E>
class Acceptable
{
	enum { All, Cyl, Sphere, Rect, Line, Frustum, Really_All } what;
	world::Sphere bsp;
	Point p1;
	Point p2;
	world::Frustum frustum;
public:
	Acceptable(bool all = false)
	 : what(all ? Really_All : All)
	{ }

	Acceptable(const world::Rect& r)
	 : what(Rect), p1(r.ll.x, 0, r.ll.y), p2(r.ur.x, 0, r.ur.y)
	{
		Point ll = Point(r.ll.x, 0, r.ll.y);
		float w = (r.ur.x - r.ll.x) / 2;
		float h = (r.ur.y - r.ll.y) / 2;
		bsp.p = ll + Point(w, 0, h);
		bsp.r = sqrt(w*w+h*h);
	}

	Acceptable(const world::Cylinder& cyl)
	 : what(Cyl), bsp(Point(cyl.p.x, 0, cyl.p.y), cyl.r)
	{ }

	Acceptable(const world::Sphere& s)
	 : what(Sphere), bsp(s)
	{ }

	Acceptable(const world::Line& l);

	Acceptable(const world::Frustum& fr)
	 : what(Frustum), frustum(fr)
	{
		//FIXME
		bsp.p = fr.pos() + fr.dir();
		bsp.r = 2*length(fr.dir());
	}

	Acceptable(const Acceptable& ac)
	 : what(ac.what), bsp(ac.bsp), p1(ac.p1), p2(ac.p2), frustum(ac.frustum)
	{ }

	Acceptable& operator=(const Acceptable& ac);

	Quadrant quadrant(Square sq) const;

	bool acceptable(const Box<E>& b) const;

	typedef NodeCont<E> NS;

	typename NS::iterator find_begin(typename NS::t& set, float max_size);

	typename NS::iterator find_end(typename NS::t& set, float max_size);
	float min_height() const;
};


template<class E>
class QuadTree_dfs_iterator
{
	Acceptable<E> ok_quads;
	typedef Node<E> Node_t;

	typedef QuadTree_dfs_iterator<E> iterator;

	QuadTree<E>* tree;
	Node_t* last;
	std::stack<Node_t*> nodes;
	typename Node_t::iterator current;
	typename Node_t::iterator cur_end;
	bool at_end, sub_iter;
	int search_id;
	int iter_id;

	void find_next_node();
	bool test();
public:
	typedef NodeCont<E> NS;

	typedef E value_type;
	typedef value_type* pointer;
	typedef value_type& reference;
	typedef std::forward_iterator_tag iterator_category;
	typedef int difference_type;
	typedef int distance_type;

	QuadTree_dfs_iterator()
	 : tree(0), last(0), at_end(true), sub_iter(false), search_id(-1), iter_id(-1)
	{ }

	QuadTree_dfs_iterator(const QuadTree_dfs_iterator& qi)
	 : ok_quads(qi.ok_quads), tree(qi.tree), last(qi.last), nodes(qi.nodes)
	 , current(qi.current), cur_end(qi.cur_end)
	 , at_end(qi.at_end), sub_iter(qi.sub_iter)
	 , search_id(qi.search_id), iter_id(qi.iter_id)
	{ }


	template<class T>
	QuadTree_dfs_iterator(QuadTree<E>* t, const T& ok, int sc, int ii)
//	 : ok_quads(t), last(0), at_end(false), sub_iter(false)
	{
		ok_quads = Acceptable<E>(ok);
		tree = t;
		last = 0;
		at_end = sub_iter = false;
		search_id = sc;
		iter_id = ii;
		nodes.push(t->root);
		if (!t->overflow->elems.empty())
			nodes.push(t->overflow);
		find_next_node();
	}

	template<class Tree>
	QuadTree_dfs_iterator(Tree* t, bool all)
	{
		*this = t->begin(all);
	}

	QuadTree_dfs_iterator(QuadTree<E>* t, bool all, int sc, int ii)
	 : ok_quads(all), tree(t), last(0), at_end(false), sub_iter(false),
	   search_id(sc), iter_id(ii)
	{
		nodes.push(t->root);
		if (!t->overflow->elems.empty())
			nodes.push(t->overflow);
		find_next_node();
	}

	~QuadTree_dfs_iterator()
	{
		if (iter_id >= 0 && tree)
			tree->iterators[iter_id] = 0;			
	}

	QuadTree_dfs_iterator& operator=(const QuadTree_dfs_iterator& qi)
	{
		ok_quads = qi.ok_quads;
		tree = qi.tree;
		last = qi.last;
		nodes = qi.nodes;
		current = qi.current;
		cur_end = qi.cur_end;
		at_end = qi.at_end;
		sub_iter = qi.sub_iter;
		search_id = qi.search_id;
		iter_id = qi.iter_id;
		return *this;
	}

	const Box<E>* deref() {
		return *current;
	}

	reference operator*() const {
		return (**current).unbox();
	}

	reference operator*() {
		return (**current).unbox();
	}

	const pointer operator->() const {
		return &(**current).unbox();
	}

	pointer operator->() {
		return &(**current).unbox();
	}

	QuadTree_dfs_iterator& operator++() {
		find_next_node();
		return *this;
	}

	QuadTree_dfs_iterator operator++(int)
	{
		QuadTree_dfs_iterator i(*this);
		++(*this);
		return i;
	}

	void erase() const { remove(); }

	void remove() const;

	bool operator==(const QuadTree_dfs_iterator& q) const {
		return (at_end && q.at_end) || (!at_end && !q.at_end && current == q.current);
	}
	bool operator!=(const QuadTree_dfs_iterator& q) const {
		return ! (*this == q);
	}
};


template<class E>
class QuadTree
{
	friend class QuadTree_dfs_iterator<E>;
	Node<E>* root;
	Node<E>* overflow;
	std::vector<int> iterators;
	int search_id;
	QuadTree(const QuadTree&);
	QuadTree operator=(const QuadTree&);

	int alloc_iter();
public:
	QuadTree(float xoff, float yoff, float sz);

	~QuadTree();

	void insert(E e);

	typedef QuadTree_dfs_iterator<E> iterator;

	iterator begin(bool all = false);

	iterator end() const;

	template<class T>
	iterator find(const T& t)
	{
		return iterator(this, t, ++search_id, alloc_iter());
	}

	void erase(const iterator& i);

	template<class T>
	float max_height(const T& t)
	{
		float height = 0;
		iterator c, e = end();
		for (c = find(t); c != e; ++c) {
			height = std::max(height, c.deref()->get_height());
		}
		return height;
	}

	void update();
	void show();
};



}
}

#endif

