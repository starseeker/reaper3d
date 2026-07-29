#ifndef REAPER_MISC_UNIQUEPTR_H
#define REAPER_MISC_UNIQUEPTR_H

#include <memory>
#include <utility>

namespace reaper {
namespace misc {

// Live handle to a process-wide service instance. Handles share the registry
// slot rather than taking a snapshot of its current value, so a handle created
// as a member before service initialization observes the instance once create()
// registers it. This preserves the service-locator semantics used throughout
// the engine without exposing a dangling raw pointer after destroy().
template<class T>
class UniquePtr
{
	struct ServiceSlot
	{
		std::shared_ptr<T> instance;
	};

	inline static std::shared_ptr<ServiceSlot> registry =
		std::make_shared<ServiceSlot>();

	std::shared_ptr<ServiceSlot> slot;

	explicit UniquePtr(std::shared_ptr<ServiceSlot> service_slot)
		: slot(std::move(service_slot))
	{
	}

public:
	UniquePtr()
		: slot(registry)
	{
	}

	template<class... Args>
	static UniquePtr create(Args&&... args)
	{
		registry->instance = std::shared_ptr<T>(
			new T(std::forward<Args>(args)...));
		return UniquePtr(registry);
	}

	static void destroy()
	{
		registry->instance.reset();
	}

	T& operator*() { return *slot->instance; }
	T* operator->() { return slot->instance.get(); }
	const T& operator*() const { return *slot->instance; }
	const T* operator->() const { return slot->instance.get(); }

	int count() const
	{
		if (!valid())
			return 0;
		return static_cast<int>(slot.use_count()) - 1;
	}

	bool valid() const
	{
		return slot && static_cast<bool>(slot->instance);
	}
	explicit operator bool() const { return valid(); }
};

} // namespace misc
} // namespace reaper

#endif
