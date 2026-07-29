#include "hw/concurrent.h"

namespace reaper::hw::concurrent
{

void Mutex::lock()
{
	mutex.lock();
}

bool Mutex::try_lock()
{
	return mutex.try_lock();
}

void Mutex::unlock()
{
	mutex.unlock();
}

Semaphore::Semaphore(std::size_t value)
	: count(value)
{
}

void Semaphore::wait()
{
	std::unique_lock lock(mutex);
	ready.wait(lock, [this] { return count != 0; });
	--count;
}

bool Semaphore::try_wait()
{
	std::lock_guard lock(mutex);
	if (count == 0)
		return false;

	--count;
	return true;
}

void Semaphore::signal()
{
	{
		std::lock_guard lock(mutex);
		++count;
	}
	ready.notify_one();
}

}
