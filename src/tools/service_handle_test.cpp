#include "misc/uniqueptr.h"

namespace {

struct Service
{
	inline static int live_instances = 0;

	explicit Service(int initial_value)
		: value(initial_value)
	{
		++live_instances;
	}

	~Service()
	{
		--live_instances;
	}

	int value;
};

using ServiceHandle = reaper::misc::UniquePtr<Service>;

} // namespace

int main()
{
	ServiceHandle early_handle;
	if (early_handle.valid())
		return 1;

	ServiceHandle owner = ServiceHandle::create(7);
	if (!early_handle.valid() || early_handle->value != 7 ||
	    Service::live_instances != 1)
		return 2;

	ServiceHandle late_handle;
	if (owner.count() != 3 || late_handle->value != 7)
		return 3;

	ServiceHandle replacement = ServiceHandle::create(9);
	if (early_handle->value != 9 || late_handle->value != 9 ||
	    Service::live_instances != 1)
		return 4;

	ServiceHandle::destroy();
	if (early_handle.valid() || owner.valid() || replacement.valid() ||
	    Service::live_instances != 0)
		return 5;

	return 0;
}
