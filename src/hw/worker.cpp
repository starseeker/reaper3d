

#include <memory>
#include <queue>
#include <thread>

#include "hw/concurrent.h"
#include "hw/abstime.h"
#include "hw/worker.h"

namespace reaper {
namespace hw {
namespace worker {

namespace cc = concurrent;

using std::pair;
using std::queue;

class Child
{
	JobPtr job;
	std::thread thread;
public:
	explicit Child(JobPtr j)
		: job(std::move(j)),
		  thread(&Child::run, this)
	{
//		printf("start\n");
	}
	~Child()
	{
		if (thread.joinable())
			thread.join();
	}
	void run()
	{
		(*job)();
	}
};

class Slave : public Worker
{
	struct SlaveJob
	{
		concurrent::Mutex jobs_mtx;

		using jobs_t = queue<JobPtr>;
		jobs_t jobs;
		cc::Semaphore quit_req;
		void run()
		{
			while (true) {
				JobPtr job;
				bool have_job = false;
				{
					concurrent::ScopeLock sc(jobs_mtx);
//				  printf("child looping, size: %d\n", jobs.size());
					if (!jobs.empty()) {
						job = std::move(jobs.front());
						jobs.pop();
						have_job = true;
					}
				}
				if (!have_job) {
					if (quit_req.try_wait())
						return;
					time::msleep(10);
					continue;
				}
				if ((*job)()) {
//					printf("again\n");
					concurrent::ScopeLock sc(jobs_mtx);
					jobs.push(std::move(job));
				} else {
					job->done();
				}
				time::msleep(10);
			}
		}
	};
	SlaveJob sj;

	std::thread thread;
	std::deque<std::unique_ptr<Child>> children;
	bool stopped = false;

public:
	Slave()
		: thread(&SlaveJob::run, &sj)
	{
	}
	virtual ~Slave()
	{
		shutdown();
	}
	void spawn_job(JobPtr job)
	{
//		printf("spawn\n");
		children.push_back(std::make_unique<Child>(std::move(job)));
	}
	void add_job(JobPtr job)
	{
//		printf("add\n");
		concurrent::ScopeLock sc(sj.jobs_mtx);
		sj.jobs.push(std::move(job));
	}
	void shutdown()
	{
		if (stopped)
			return;
		stopped = true;

//		printf("jobs left: %d, children %d\n", sj.jobs.size(), children.size());
		sj.quit_req.signal();
		if (thread.joinable())
			thread.join();
		children.clear();
	}
};

Worker* worker()
{
	static Slave slave;
	return &slave;
}


}
}
}
