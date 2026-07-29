
/* $Id: worker.h,v 1.8 2002/01/17 04:58:47 peter Exp $ */

#ifndef REAPER_HW_WORKER_H
#define REAPER_HW_WORKER_H

#include <memory>


namespace reaper {
namespace hw {
namespace worker {

struct Job
{
	virtual bool operator()() = 0;
	virtual void done() { }
	virtual ~Job() { }
};

using JobPtr = std::shared_ptr<Job>;

class Worker
{
public:
	virtual void spawn_job(JobPtr) = 0;
	virtual void add_job(JobPtr) = 0;
	virtual void shutdown() = 0;
	virtual ~Worker() = default;
};

Worker* worker();

}
}
}

#endif
