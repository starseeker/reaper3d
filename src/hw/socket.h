#ifndef REAPER_HW_SOCKET_H
#define REAPER_HW_SOCKET_H

#include <cstdint>
#include <deque>
#include <memory>
#include <string>

#include "hw/exceptions.h"

namespace reaper::hw::net
{

struct socket_error : public hw_error {
	explicit socket_error(const std::string& message)
		: hw_error(message)
	{
	}
};

using socket_t = int;
using addr_t = std::uint32_t;

std::string addr2string(addr_t address);

class Net_Init {
	static int instances;

public:
	Net_Init();
	~Net_Init();
	Net_Init(const Net_Init&) = delete;
	Net_Init& operator=(const Net_Init&) = delete;
};

class ConnData;

/** IPv4 TCP/UDP socket. */
class Socket {
	socket_t socket;
	std::unique_ptr<ConnData> data;

	void init_socket();
	explicit Socket(socket_t accepted_socket);

public:
	enum Proto { TCP, UDP };

	Socket();
	~Socket();
	Socket(const Socket&) = delete;
	Socket& operator=(const Socket&) = delete;

	int getfd() const;
	void set_proto(Proto protocol);
	void set_local_params(addr_t address, long port);
	void set_remote_params(addr_t address, long port);
	addr_t local_addr() const;
	addr_t remote_addr() const;
	long remote_port() const;

	void connect(addr_t address = 0, long port = 0, Proto protocol = TCP);
	void listen(long port);
	std::unique_ptr<Socket> accept();
	void close();

	int send(const char* buffer, std::size_t length);
	int send(
		const char* buffer,
		std::size_t length,
		addr_t address,
		long port);
	int recv(char* buffer, std::size_t length, bool block = true);
	int recv(
		char* buffer,
		std::size_t length,
		addr_t* address,
		long* port,
		bool block = true);
};

using SSeq = std::deque<Socket*>;

bool wait_read(
	const SSeq& input,
	SSeq& readable,
	SSeq& errors,
	int timeout = -1);

}

#endif
