#ifndef REAPER_NET_NAMESERVICE_H
#define REAPER_NET_NAMESERVICE_H

#include <list>
#include <memory>
#include <string>

#include "hw/socket.h"
#include "main/exceptions.h"

namespace reaper::net
{

using hw::net::addr_t;

/// Result of a name-service query.
struct NameData {
	bool err = false;
	std::string name;
	std::list<addr_t> addrs;
	std::list<std::string> aliases;
};

class NSHelper;

/// Asynchronous domain-name lookup service.
class NameService {
	std::unique_ptr<NSHelper> helper;

	NameService();

public:
	~NameService();
	NameService(const NameService&) = delete;
	NameService& operator=(const NameService&) = delete;

	static NameService* Instance();
	void Lookup(const std::string& host);
	void Lookup(addr_t address);
	bool Result(bool wait, NameData&);
};

class network_error : public error_base {
	std::string err;

public:
	explicit network_error(const std::string& message)
		: err(message)
	{
	}

	const char* what() const noexcept override { return err.c_str(); }
	~network_error() override = default;
};

}

#endif
