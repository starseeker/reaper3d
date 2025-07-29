/*
 * $Author: pstrand $
 * $Date: 2002/09/13 07:45:57 $
 * $Log: quadtree.cpp,v $
 * Revision 1.74  2002/09/13 07:45:57  pstrand
 * gcc-varningar
 *
 * Revision 1.73  2002/06/22 12:08:56  pstrand
 * hmm
 *
 * Revision 1.72  2002/06/06 07:56:28  pstrand
 * *** empty log message ***
 *
 * Revision 1.71  2002/05/22 21:32:38  pstrand
 * lazy delete
 *
 * Revision 1.70  2002/05/21 10:01:21  pstrand
 * *** empty log message ***
 *
 * Revision 1.69  2002/05/06 19:47:10  fizzgig
 * moved derr-definition.. vc complained
 *
 * Revision 1.68  2002/05/04 23:10:50  pstrand
 * *** empty log message ***
 *
 * Revision 1.67  2002/05/04 11:12:37  pstrand
 * *** empty log message ***
 *
 * Revision 1.66  2002/04/30 10:53:04  pstrand
 * errmsg
 *
 * Revision 1.65  2002/04/29 22:26:37  pstrand
 * *** empty log message ***
 *
 * Revision 1.64  2002/04/29 21:19:35  pstrand
 * const erase
 *
 * Revision 1.63  2002/04/29 20:43:19  pstrand
 * shots i quadtree, upprensning&buggfixar..
 *
 * Revision 1.62  2002/04/29 20:33:26  fizzgig
 * Added quadtree for particle-systems, plus shot->silly fix
 *
 * Revision 1.61  2002/04/19 17:50:16  pstrand
 * fix
 *
 * Revision 1.60  2002/04/15 16:15:55  pstrand
 * linjes�kningsfix
 *
 * Revision 1.59  2002/04/14 21:18:53  pstrand
 * n�t
 *
 * Revision 1.58  2002/04/14 18:37:28  pstrand
 * *** empty log message ***
 *
 * Revision 1.57  2002/04/14 16:43:20  pstrand
 * sort
 *
 * Revision 1.56  2002/04/14 09:22:37  pstrand
 * quadtree redesign and Sphere::radius -> Sphere::r
 *
 * Revision 1.55  2002/04/12 18:51:13  pstrand
 * dynobj i quadtree
 *
 * Revision 1.54  2002/04/09 03:36:32  pstrand
 * eff.ptr koll, shot-parent-id, quadtree-update-update
 *
 * Revision 1.53  2002/04/06 22:25:33  pstrand
 * win32fixar
 *
 * Revision 1.52  2002/04/06 20:10:07  pstrand
 * quadtree dynamicification ;)
 *
 * Revision 1.51  2002/03/24 17:40:06  pstrand
 * no message
 *
 * Revision 1.50  2002/01/31 17:40:00  peter
 * tja..
 *
 * Revision 1.49  2002/01/31 05:36:04  peter
 * *** empty log message ***
 *
 * Revision 1.48  2002/01/31 03:41:33  peter
 * fixad remove
 *
 * Revision 1.47  2002/01/31 03:20:34  peter
 * fixad remove
 *
 */


#include "hw/compat.h"
#include "hw/debug.h"
#include "main/types.h"
#include "main/types_io.h"
#include "misc/sequence.h"
#include "misc/smartptr.h"
#include "world/geometry.h"
#include "world/quadtree.h"
#include "gfx/light.h"
#include "gfx/abstracts.h"
#include "object/base.h"
#include "misc/free.h"

#include <limits>
#include <iostream>
#include <algorithm>

namespace reaper
{
namespace { debug::DebugOutput derr("qt", 5); }

namespace quadtree
{

const int Out = 0, LL = 1, LR = 2, UL = 4, UR = 8, All = 15;


Square Square::shrinked(Quadrant q) const
{
	Square sq(xoff, yoff, half_size);
	switch (q) {
	case LL:  break;
	case LR:  sq.xoff += half_size; break;
	case UR:  sq.xoff += half_size;
	case UL:  sq.yoff += half_size; break;
	}
	sq.half_size /= 2;
	return sq;
}

Square Square::grown() const
{
	Square sq(xoff, yoff, half_size);
	sq.half_size *= 2;
	sq.xoff -= sq.half_size;
	sq.yoff -= sq.half_size;
	sq.half_size *= 2;
	return sq;
}

Quadrant get_quadrant(const Point2D& ll, const Point2D& ur, const Square& sq)
{
	float sz = sq.half_size;
	float sz2 = sz*2;
	float ll_x = ll.x - sq.xoff;
	float ur_x = ur.x - sq.xoff;
	float ll_y = ll.y - sq.yoff;
	float ur_y = ur.y - sq.yoff;

	Quadrant q = Out;

	if (ur_x >= 0 && ur_y >= 0 && ll_x < sz && ll_y < sz)
		q |= LL;

	if (ur_x >= 0 && ur_y >= sz && ll_x < sz && ll_y < sz2)
		q |= UL;

	if (ur_x >= sz && ur_y >= 0 && ll_x < sz2 && ll_y < sz)
		q |= LR;

	if (ur_x >= sz && ur_y >= sz && ll_x < sz2 && ll_y < sz2)
		q |= UR;

	return q;
}

Quadrant get_quadrant(const Point& p1, const Point& p2, const Square& sq)
{
	return get_quadrant(Point2D(p1.x, p1.z), Point2D(p2.x, p2.z), sq);
}

Quadrant get_quadrant(Point2D c, float r, Square sq)
{
	return get_quadrant(Point2D(c.x - r, c.y - r),
			    Point2D(c.x + r, c.y + r), sq);
}

Quadrant get_quadrant(Point c, float r, Square sq)
{
	return get_quadrant(Point2D(c.x - r, c.z - r),
			    Point2D(c.x + r, c.z + r), sq);
}



/** Quadrant-calculator for a SillyObject
 *  (or any such object) with a bounding sphere.
 */

template<class Elem>
struct Geom
{
	static float get_height(Elem e)
	{
		return e->get_pos().y + e->get_radius();
	}
	static Quadrant quadrant(Elem e, const Square& sq)
	{
		float r = e->get_radius();
		Point pos = e->get_pos();
		return get_quadrant(pos, r, sq);
	}

	static bool intersect(const Box<Elem>& b, const Point& center, float radius)
	{
		return world::intersect(b.pos(), b.radius(), center, radius);
	}


	static bool intersect(const Box<Elem>& b, const Point2D& center, float radius)
	{
		Point2D pos(b.pos().x, b.pos().z);
		return world::intersect(pos, b.radius(), center, radius);
	}

	static bool intersect(const Box<Elem>& b, const Point2D& ll, const Point2D& ur)
	{
		Point p = b.pos();
		float r = b.radius();
		return world::intersect(ll, ur, Point2D(p.x - r, p.z - r), Point2D(p.x + r, p.z + r));
	}

	// FIXME
	static bool intersect(const Box<Elem>& b, const world::Line& l)
	{
		Point p = b.pos();
		float r = b.radius();
		return world::intersect(Point(p.x - r, p.y - r, p.z - r),
					   Point(p.x + r, p.y + r, p.z + r), l);
	}

	static bool intersect(const Box<Elem>& b, const world::Frustum& f)
	{
		return world::intersect(b.pos(), b.radius(), f);
	}

	static world::Sphere calc_bsphere(Elem e)
	{
		return world::Sphere(e->get_pos(), e->get_radius());
	}

};


using world::mintri;
using world::maxtri;
using world::mintri2;
using world::maxtri2;

/** Quad-helper for triangles */

template <>
struct Geom<world::Triangle*>
{
	typedef world::Triangle Tri;
	static float get_height(Tri* t)
	{
		return world::max(t->a.y, t->b.y, t->c.y);
	}
	

	static Quadrant quadrant(Tri* t, const Square& sq)
	{
		Quadrant q = quadtree::get_quadrant(mintri(t->a, t->b, t->c),
					      maxtri(t->a, t->b, t->c), sq);
		return quadtree::get_quadrant(mintri(t->a, t->b, t->c),
					      maxtri(t->a, t->b, t->c), sq);
	}

	static bool intersect(const Box<Tri*>& b, const Point& center, float radius)
	{
		return world::intersect(b.pos(), b.radius(), center, radius);
	}

	static bool intersect(const Box<Tri*>& b, const Point2D& center, float radius)
	{
		return world::intersect(Point2D(b.pos().x, b.pos().z), b.radius(), center, radius);
	}

	static bool intersect(const Box<Tri*>& b, const Point2D& ll, const Point2D& ur)
	{
		return world::intersect(ll, ur, Point2D(b.pos().x, b.pos().z), b.radius());
	}

	static bool intersect(const Box<Tri*>& b, const world::Frustum& f)
	{
		return world::intersect(*b.unbox(), f);
	}

	static bool intersect(const Box<Tri*>& b, const world::Line& l)
	{
		return world::intersect(b.pos(), b.radius(), l);
	}

	static world::Sphere calc_bsphere(Tri* t)
	{
		const Point t1(mintri(t->a, t->b, t->c));
		const Point t2(maxtri(t->a, t->b, t->c));
		const Point rad = (t2 - t1) / 2.0;
		return world::Sphere(t1 + rad, length(rad));
	}

};

template<class E>
Acceptable<E>& Acceptable<E>::operator=(const Acceptable<E>& ac)
{
	what = ac.what;
	bsp = ac.bsp;
	p1 = ac.p1;
	p2 = ac.p2;
	frustum = ac.frustum;
	return *this;
}

template<class E>
Quadrant Acceptable<E>::quadrant(Square sq) const {
	switch (what) {
	case Really_All:
	case All:     return LL+LR+UL+UR;
	case Rect:    return get_quadrant(p1, p2, sq);
	case Cyl:
	case Frustum:
	case Sphere:  return get_quadrant(bsp.p, bsp.r, sq);
	case Line:    return get_quadrant(world::minpt(p1, p2), world::maxpt(p1, p2), sq);
	}
	return LL+LR+UL+UR;
}

bool is_dead(const Box<object::SillyPtr>& p)
{
	return p.unbox()->is_dead();
}

bool is_dead(const Box<object::StaticPtr>& p)
{
	return p.unbox()->is_dead();
}

bool is_dead(const Box<object::DynamicPtr>& p)
{
	return p.unbox()->is_dead();
}

bool is_dead(const Box<object::ShotPtr>& p)
{
	return p.unbox()->is_dead();
}

template<class C>
bool is_dead(const C&)
{
	return false;
}


template<class E>
bool Acceptable<E>::acceptable(const Box<E>& b) const {
	if (what == Really_All)
		return true;
	if (is_dead(b))
		return false;
	switch (what) {
	case All:     return true;
	case Rect:    return Geom<E>::intersect(b, Point2D(p1.x, p1.z), Point2D(p2.x, p2.z));
	case Cyl:     return Geom<E>::intersect(b, Point2D(bsp.p.x, bsp.p.z), bsp.r);
	case Sphere:  return Geom<E>::intersect(b, bsp.p, bsp.r);
	case Line:    return Geom<E>::intersect(b, world::Line(p1, p2));
	case Frustum: return Geom<E>::intersect(b, frustum);
	case Really_All: break;
	}
	return false;
}

template<class E>
typename Acceptable<E>::NS::iterator Acceptable<E>::find_begin(typename NS::t& set, float max_size)
{
	if (what != Really_All && what != All) {
		float d = 2*bsp.r + 2*max_size;
		Box<E> box(bsp.p.x - d, bsp.p.z - d);
		return NS::lower_bound(set, &box);
	} else {
		return set.begin();
	}
}

template<class E>
typename Acceptable<E>::NS::iterator Acceptable<E>::find_end(typename NS::t& set, float max_size)
{
	if (what != Really_All && what != All) {
		float d = 2*bsp.r + 2*max_size;
		Box<E> box(bsp.p.x + d, bsp.p.z + d);
		return NS::upper_bound(set, &box);
	} else {
		return set.end();
	}
}

template<class E>
float Acceptable<E>::min_height() const {
	switch (what) {
	case Frustum:
	case Sphere:
		return bsp.p.y - bsp.r;
	case Line:
		return std::min(p1.y, p2.y);
	default:
		return -1e20; // FIXME
	}
}

template<class E>
Acceptable<E>::Acceptable(const world::Line& l)
 : what(Line), p1(l.p1), p2(l.p2)
{
	Point rad((p2-p1)/2);
	bsp.p = p1 + rad;
	bsp.r = length(rad);
}

template<class Iter, class T>
inline void isort1(Iter first, Iter last, T)
{
	for (Iter i = first + 1; i != last; ++i) {

		T val = *i;
		if (val < *first) {
			copy_backward(first, i, i + 1);
			*first = val;
		}
		else
		{
			Iter lst = i;
			Iter next = i;
			--next;
			while (val < *next) {
				*lst = *next;
				lst = next;
				--next;
			}
			*lst = val;
		}
	}
}

template<class Iter, class T>
inline void isort2(Iter first, Iter last, T)
{
	for (Iter i = first + 1; i != last; ++i) {
		T x = *i;
		Iter j = i-1;
		while (j >= first && x < *j) {
			*(j+1) = *j;
			--j;
		}
		*(j+1) = x;
	}
}

template <class Iter>
void qt_sort(Iter first, Iter last)
{
	if (first == last) return; 
	isort2(first, last, *first);
}


/*
template<class Iter >
void qt_sort(Iter first, Iter last)
{
	std::sort(first, last);
}
*/


template<class E>
bool QuadTree_dfs_iterator<E>::test() {
	Box<E>* b = *current;
	if (b->elem == 0 || b->count == 0) {
//		if (!b->deleted)
//			derr << "Null entry found in tree!\n";
		return false;
	}
	if (!b->deleted && ok_quads.acceptable(*b)) {
		if (b->count > 1) {
			if (!(b->visitors[iter_id] == search_id)) {
				b->visitors[iter_id] = search_id;
				return true;
			}
		} else {
			return true;
		}
	}
	return false;
}

template<class E>
void QuadTree_dfs_iterator<E>::find_next_node() {
	while (!nodes.empty()) {
		Node_t* p = nodes.top();
		last = p;

		if (ok_quads.min_height() > p->max_height) {
			nodes.pop();
			continue;
		}
		
		switch (p->state) {
		case Node_t::IntNode: {
			nodes.pop();
			Quadrant acpt = ok_quads.quadrant(p->geom);
			if (acpt & LL) nodes.push(p->quad[0]);
			if (acpt & LR) nodes.push(p->quad[1]);
			if (acpt & UL) nodes.push(p->quad[2]);
			if (acpt & UR) nodes.push(p->quad[3]);
//			if (!acpt) derr << "find_next_node: warning, outside quad\n";
			continue;
		}
		case Node_t::TmpLeaf:
		case Node_t::Leaf: {
			bool sub_at_end = false;
			if (! p->elems.empty()) {
				if (! sub_iter) {
					current = ok_quads.find_begin(p->elems, p->max_size);
					cur_end = ok_quads.find_end(p->elems, p->max_size);
					if (current == cur_end) {
						sub_at_end = true;
					} else {
						sub_iter = true;
						if (test())
							return;
					}
				} else {
					++current;
					if (current != cur_end) {
						if (test())
							return;
					} else {
						sub_at_end = true;
						sub_iter = false;
					}
				}
			}
			if (sub_at_end || p->elems.empty()) {
				nodes.pop();
			}
			break;
		}
		}
	}
	at_end = true;
}

template<class E>
void QuadTree_dfs_iterator<E>::remove() const
{
	if (current == last->elems.end()) {
//		derr << "remove: warning, end-iterator\n";
		return;
	}
	Box<E>* b = *current;
	b->deleted = true;
//	b->elem = 0;
/*
	Box<E>* b = *current;
	b->count--;
	if (b->count == 0)
		delete b;
	else
		b->clear();
	last->elems.erase(current);
*/
}

template<class E>
bool Node<E>::empty() const
{
	return (state == Leaf) ? elems.empty()
		: (quad[0]->empty() && quad[1]->empty() &&
		   quad[2]->empty() && quad[3]->empty());
}


template<class E>
void Node<E>::update(Node<E>* root)
{
	if (state == IntNode) {
		for (int i = 0; i < 4; ++i)
			quad[i]->update(root);
	} else {
		std::vector<Box<E>*> tmp;
		typename NodeCont<E>::t stay;

		Point2D ll(geom.xoff, geom.yoff);
		Point2D ur(ll + Point2D(2*geom.half_size, 2*geom.half_size));

		iterator c, e = NS::end(elems);
		for (c = NS::begin(elems); c != e; ++c) {
			if (!(*c)->deleted) {
				(*c)->update(Geom<E>::calc_bsphere((*c)->unbox()));
				if (Geom<E>::intersect(**c, ll, ur))
					NS::insert(stay, *c);
				else
					tmp.push_back(*c);
			}
		}
		std::swap(elems, stay);
//		elems.clear();
//		NS::sort(elems);
		for (size_t i = 0; i < tmp.size(); ++i) {
			Box<E>* b = tmp[i];
			bool del = b->count == 1;
			Box<E>* b2 = b->spawn();
			if (del)
				delete b;
			root->insert(b2);
		}
			
	}
}

void indent(int n) {
	for (int i = 0; i < n; ++i)
		putchar(' ');
}

template<class T>
Point get_pos(misc::SmartPtr<T> p) {
	return p->get_pos();
}

Point get_pos(gfx::light::Light*) {
	return Point();
}

Point get_pos(world::Triangle* t) {
	return world::midpoint(mintri(t->a, t->b, t->c), maxtri(t->a, t->b, t->c));
}

template<class T>
float radius(misc::SmartPtr<T> p) {
	return p->get_radius();
}

float radius(gfx::light::Light* t) {
	return 0;
}

float radius(world::Triangle* t) {
	return length(mintri(t->a, t->b, t->c) - maxtri(t->a, t->b, t->c)) / 2.0;
}

template<class E>
void Node<E>::show(int level)
{
/*
	indent(level); printf("Node %d Geometry (%f,%f) %f\n", state, geom.xoff, geom.yoff, geom.half_size);
	if (state == IntNode) {
		for (int i = 0; i < 4; ++i)
			quad[i]->show(level+2);
	} else {
		indent(level); printf("Elements (%4d): ", elems.size());
		iterator c, e = NS::end(elems);
		for (c = NS::begin(elems); c != e; ++c) {
			if ((*c)->deleted)
				continue;
			Point p = get_pos((*c)->unbox());
			printf(" (%d|%d) (%f, %f, %f | %f) \n",
				(*c)->count, (*c)->deleted, p.x, p.y, p.z, radius((*c)->unbox()));
			indent(level+17); 
		}
		putchar('\n');
	}
*/
}

template<class T> struct QuadTreeInfo { };

using std::max;
using std::min;

template<class E>
Node<E>::Node(const Square& sq, int d)
 : geom(sq), max_size(0), max_height(-1), depth(d)
{
	state = (depth < QuadTreeInfo<E>::max_depth) ? TmpLeaf : Leaf;
	quad[0] = quad[1] = quad[2] = quad[3] = 0;
}

template<class E>
Node<E>::~Node()
{
	for (int i = 0; i < 4; ++i) {
		delete quad[i];
	}
	for (size_t i = 0; i < elems.size(); ++i) {
		if (--elems[i]->count == 0)
			delete elems[i];
	}
}

template<class S>
void ns_clear(S& s)
{
	typename S::iterator c, e = s.end();
	for (c = s.begin(); c != e; ++c) {
		if ((**c).count == 1)
			delete *c;
	}
	s.clear();
}


template<class NS, class T, class B>
void insert_to_leaf(T& elems, B b)
{
	if (std::find(elems.begin(), elems.end(), b) == elems.end()) {
		b->count++;
		NS::insert(elems, b);
	}
}

template<class E>
void Node<E>::insert(Box<E>* b) {
	max_size = max(b->radius(), max_size);
	max_height = max(b->get_height(), max_height);
	switch (state) {
	case Leaf:
		insert_to_leaf<NS>(elems, b);
		break;
	case TmpLeaf:
		if (elems.size() < QuadTreeInfo<E>::max_tmp_elems) {
			insert_to_leaf<NS>(elems, b);
		} else {
			state = IntNode;
			for (int i = 0; i < 4; ++i)
				quad[i] = new Node(geom.shrinked(1<<i), depth+1);
			insert(b);
			iterator c, e = NS::end(elems);
			for (c = NS::begin(elems); c != e; ++c) {
				if (!(*c)->deleted) {
					(*c)->count--;
					insert(*c);
				}
			}
			elems.clear();
		}
		break;
	case IntNode:
		Quadrant q = Geom<E>::quadrant(b->unbox(), geom);
		if (q & LL) quad[0]->insert(b);
		if (q & LR) quad[1]->insert(b);
		if (q & UL) quad[2]->insert(b);
		if (q & UR) quad[3]->insert(b);
		if (!q && !b->count) derr << "insert: warning, element outside tree\n";
		break;
	}
}

template<class E>
void QuadTree<E>::insert(E e)
{
	Box<E>* b = new Box<E>(Geom<E>::calc_bsphere(e), e);
	root->insert(b);
	if (b->count == 0)
		overflow->insert(b);
}

template<class E>
QuadTree<E>::QuadTree(float xoff, float yoff, float sz)
 : search_id(0)
{
	root = new Node<E>(Square(xoff,yoff,sz/2), 0);
	float d_max = std::numeric_limits<float>::max();
	float d_min = -d_max;
	overflow = new Node<E>(Square(d_min, d_min, d_max),
			       QuadTreeInfo<E>::max_depth);
}

template<class E>
QuadTree<E>::~QuadTree()
{
	delete root;
	delete overflow;
}


template<class E>
int QuadTree<E>::alloc_iter()
{
	size_t i = 0;
	for (; i < iterators.size(); i++) {
		if (!iterators[i]) {
			iterators[i] = 1;
			return i;
		}
	}
	iterators.push_back(i);
	return i;

}

template<class E>
typename QuadTree<E>::iterator QuadTree<E>::begin(bool all)
{
	int iter_id = alloc_iter();
	if (iter_id > 3)
		throw reaper_error("no more than four searches are allowed at the same time!");
	return iterator(this, all, ++search_id, iter_id);
}

template<class E>
typename QuadTree<E>::iterator QuadTree<E>::end() const
{
	return iterator();
}


template<class E>
void QuadTree<E>::erase(const iterator& i)
{
	if (i != iterator())
		i.remove();
	else
		derr << "Warning, erase on end iterator\n";
}

template<class E>
void QuadTree<E>::update()
{
	root->update(root);
}

template<class E>
void QuadTree<E>::show()
{
	printf("\nQuadTree: \n");
	root->show(2);
}

#define INST_QUADTREE(T, N, M) \
	template class QuadTree<T>; \
	template class Acceptable<T>; \
	template class Node<T>; \
	template class QuadTree_dfs_iterator<T>; \
	template <> \
	struct QuadTreeInfo<T> { \
		enum { max_depth = N }; \
		enum { max_tmp_elems = M }; \
	};

INST_QUADTREE(world::Triangle*, 6, 1);
INST_QUADTREE(object::SillyPtr, 5, 4);
INST_QUADTREE(object::StaticPtr, 4, 4);
INST_QUADTREE(object::DynamicPtr, 4, 1);
INST_QUADTREE(object::ShotPtr, 4, 1);
INST_QUADTREE(gfx::light::Light*, 3, 8);
INST_QUADTREE(gfx::SimEffect*, 3, 8);


}
}

