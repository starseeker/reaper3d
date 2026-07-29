#ifndef REAPER_HW_CONCURRENT_H
#define REAPER_HW_CONCURRENT_H

#include <condition_variable>
#include <cstddef>
#include <mutex>

namespace reaper::hw::concurrent
{

/// Mutex satisfying the standard BasicLockable requirements.
class Mutex {
	std::mutex mutex;

public:
	Mutex() = default;
	~Mutex() = default;
	Mutex(const Mutex&) = delete;
	Mutex& operator=(const Mutex&) = delete;

	void lock();
	bool try_lock();
	void unlock();
};

using ScopeLock = std::lock_guard<Mutex>;

/// Counting semaphore implemented with C++17 synchronization primitives.
class Semaphore {
	std::mutex mutex;
	std::condition_variable ready;
	std::size_t count;

public:
	explicit Semaphore(std::size_t value = 0);
	~Semaphore() = default;
	Semaphore(const Semaphore&) = delete;
	Semaphore& operator=(const Semaphore&) = delete;

	void wait();
	bool try_wait();
	void signal();
};

}

#endif
