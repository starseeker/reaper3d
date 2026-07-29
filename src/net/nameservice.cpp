#include "net/nameservice.h"

#include <atomic>
#include <cstdint>
#include <cstring>
#include <thread>
#include <utility>

#include "hw/concurrent.h"
#include "hw/netcompat.h"

namespace reaper::net
{

namespace cc = hw::concurrent;

class NSHelper {
	std::thread thread;
	cc::Semaphore finished;
	cc::Semaphore lookup_requested;
	cc::Mutex lookup_mutex;
	std::atomic_bool stopping{false};

	std::string host;
	addr_t address = 0;
	NameData result;

public:
	NSHelper()
		: thread(&NSHelper::run, this)
	{
	}

	~NSHelper()
	{
		stopping.store(true);
		lookup_requested.signal();
		if (thread.joinable())
			thread.join();
	}

	void run()
	{
		hw::net::Net_Init net_init;

		for (;;) {
			lookup_requested.wait();
			if (stopping.load())
				return;

			std::string lookup_host;
			addr_t lookup_address;
			{
				cc::ScopeLock lock(lookup_mutex);
				lookup_host = host;
				lookup_address = address;
			}

			hostent* entry = nullptr;
			if (lookup_address == 0) {
				entry = gethostbyname(lookup_host.c_str());
			} else {
				const auto ipv4 = static_cast<std::uint32_t>(lookup_address);
				entry = gethostbyaddr(
					reinterpret_cast<const char*>(&ipv4),
					sizeof(ipv4),
					AF_INET);
			}

			NameData lookup_result;
			if (entry == nullptr) {
				lookup_result.err = true;
				lookup_result.name = hw::net::dnsstrerror();
			} else {
				lookup_result.name = entry->h_name;

				for (char** alias = entry->h_aliases; *alias != nullptr; ++alias)
					lookup_result.aliases.emplace_back(*alias);

				for (char** raw = entry->h_addr_list; *raw != nullptr; ++raw) {
					std::uint32_t ipv4 = 0;
					std::memcpy(&ipv4, *raw, sizeof(ipv4));
					lookup_result.addrs.push_back(static_cast<addr_t>(ipv4));
				}
			}

			result = std::move(lookup_result);
			finished.signal();
		}
	}

	void lookup(addr_t value)
	{
		{
			cc::ScopeLock lock(lookup_mutex);
			host.clear();
			address = value;
		}
		lookup_requested.signal();
	}

	void lookup(const std::string& value)
	{
		{
			cc::ScopeLock lock(lookup_mutex);
			host = value;
			address = 0;
		}
		lookup_requested.signal();
	}

	bool take_result(bool wait, NameData& output)
	{
		if (wait)
			finished.wait();
		else if (!finished.try_wait())
			return false;

		if (result.err)
			throw network_error(result.name);

		output = result;
		return true;
	}
};

NameService::NameService()
	: helper(std::make_unique<NSHelper>())
{
}

NameService::~NameService() = default;

NameService* NameService::Instance()
{
	static NameService service;
	return &service;
}

bool NameService::Result(bool wait, NameData& result)
{
	return helper->take_result(wait, result);
}

void NameService::Lookup(addr_t address)
{
	helper->lookup(address);
}

void NameService::Lookup(const std::string& host)
{
	helper->lookup(host);
}

}
