#ifndef REAPER_MISC_UNIQUEPTR_H
#define REAPER_MISC_UNIQUEPTR_H

#include <memory>
#include <utility>

namespace reaper {
namespace misc {

// Shared handle to a process-wide service instance. The historical name is
// retained for source compatibility, but ownership is now explicit and safe:
// the registry owns one shared reference and every handle owns another.
template<class T>
class UniquePtr
{
	std::shared_ptr<T> ptr;
	inline static std::shared_ptr<T> instance;

	explicit UniquePtr(std::shared_ptr<T> service)
		: ptr(std::move(service))
	{
	}

public:
	UniquePtr()
		: ptr(instance)
	{
	}

	template<class... Args>
	static UniquePtr create(Args&&... args)
	{
		instance = std::shared_ptr<T>(
			new T(std::forward<Args>(args)...));
		return UniquePtr(instance);
	}

	static void destroy()
	{
		instance.reset();
	}

	T& operator*() { return *ptr; }
	T* operator->() { return ptr.get(); }
	const T& operator*() const { return *ptr; }
	const T* operator->() const { return ptr.get(); }

	int count() const
	{
		if (!ptr)
			return 0;
		const bool registry_owns_same_instance = instance == ptr;
		return static_cast<int>(ptr.use_count()) -
		       (registry_owns_same_instance ? 1 : 0);
	}

	bool valid() const { return static_cast<bool>(ptr); }
	explicit operator bool() const { return valid(); }
};

} // namespace misc
} // namespace reaper

#endif
