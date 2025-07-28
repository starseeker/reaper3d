
/* $Id: worker.h,v 1.8 2002/01/17 04:58:47 peter Exp $ */

#ifndef REAPER_HW_WORKER_H
#define REAPER_HW_WORKER_H

#include "misc/stlhelper.h"


namespace reaper {
namespace hw {
namespace worker {

struct Job
{
	virtual bool operator()() = 0;
	virtual void done() { }
	virtual ~Job() { }
};

class Worker
{
public:
	virtual void spawn_job(Job*) = 0;
	virtual void add_job(Job*, bool delete_when_done = false) = 0;
	virtual void shutdown() = 0;
};

Worker* worker();

}
}
}

#endif

