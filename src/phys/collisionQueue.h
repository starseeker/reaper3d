#ifndef PHYS_COLLISIONQUEUE_H
#define PHYS_COLLISIONQUEUE_H

#include "pairs.h"

#include <memory>
#include <queue>
#include <vector>

namespace reaper {
typedef int objId;

namespace phys {

using PairPtr = std::shared_ptr<Pair>;

class LaterCollision
{
public:
	bool operator()(const PairPtr& lhs, const PairPtr& rhs) const
	{
		return lhs->larger_than(*rhs);
	}
};

using PriorityQueue =
	std::priority_queue<PairPtr, std::vector<PairPtr>, LaterCollision>;

	}
}

#endif 
