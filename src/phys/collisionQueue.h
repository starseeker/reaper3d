#ifndef PHYS_COLLISIONQUEUE_H
#define PHYS_COLLISIONQUEUE_H

#include "phys/treap.cpp"
#include "pairs.h"
#include <queue>
#include "misc/smartptr.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace reaper {
typedef int objId;
typedef reaper::misc::SmartPtr<reaper::phys::Pair> pairPtr;

namespace phys {

class less_simtime
{
public:
	explicit less_simtime() {}
	bool operator()(const Pair* p1,const Pair* p2) const {return p1->less_than(*p2);}
};

class larger_simtime
{
public:
	explicit larger_simtime() {}
	bool operator()(const Pair* p1,const Pair* p2) const {return p1->larger_than(*p2);}
};

class smaller
{
public:
	explicit smaller() {}
	bool operator()(const Pair* p1,const Pair* p2) const {return *p1 < *p2;}
};

class equal
{
public:
	explicit equal() {}
	bool operator()(const Pair* p1,const Pair* p2) const {return *p1 == *p2;}
};

class tstId 
{
	objId id1,id2;
public:
	explicit tstId(objId i) : id1(i) , id2(0) {}
	explicit tstId(objId i1, objId i2) : id1(i1), id2(i2) {}
	bool operator()(const Pair* p) const {return p->has_id(id1) || p->has_id(id2); }

};

typedef reaper::phys::Treap<Pair*,less_simtime,tstId,smaller,equal> CollisionQueue;
typedef std::priority_queue<Pair*,std::vector<Pair*>,larger_simtime> PriorityQueue;

	}
}

#endif 
