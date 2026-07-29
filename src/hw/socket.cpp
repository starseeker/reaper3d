#include "hw/socket.h"

#include <algorithm>
#include <array>
#include <cstring>

#include "hw/netcompat.h"

namespace reaper::hw::net
{

namespace {

using SocketAddress = sockaddr_in;

sockaddr* as_sockaddr(SocketAddress& address)
{
	return reinterpret_cast<sockaddr*>(&address);
}

const sockaddr* as_sockaddr(const SocketAddress& address)
{
	return reinterpret_cast<const sockaddr*>(&address);
}

void throw_socket_error(const std::string& operation)
{
	throw socket_error(operation + ": " + sockstrerror());
}

}

#ifdef _WIN32

int Net_Init::instances = 0;

Net_Init::Net_Init()
{
	if (++instances > 1)
		return;

	WSADATA data{};
	const int error = WSAStartup(MAKEWORD(2, 2), &data);
	if (error != 0) {
		--instances;
		throw socket_error(sockstrerror(error));
	}
}

Net_Init::~Net_Init()
{
	if (--instances == 0)
		WSACleanup();
}

#else

Net_Init::Net_Init() = default;
Net_Init::~Net_Init() = default;

#endif

class ConnData {
public:
	SocketAddress local{};
	SocketAddress remote{};
	Socket::Proto protocol = Socket::TCP;
	Net_Init network;

	ConnData()
	{
		reset();
	}

	void reset()
	{
		local = {};
		remote = {};
		local.sin_family = AF_INET;
		remote.sin_family = AF_INET;
	}
};

Socket::Socket()
	: socket(-1),
	  data(std::make_unique<ConnData>())
{
	init_socket();
}

Socket::Socket(socket_t accepted_socket)
	: socket(accepted_socket),
	  data(std::make_unique<ConnData>())
{
	socket_length length = sizeof(SocketAddress);
	if (net_error(::getsockname(
		    socket,
		    as_sockaddr(data->local),
		    &length)))
		throw_socket_error("getsockname");

	length = sizeof(SocketAddress);
	if (net_error(::getpeername(
		    socket,
		    as_sockaddr(data->remote),
		    &length)))
		throw_socket_error("getpeername");
}

Socket::~Socket()
{
	close();
}

void Socket::init_socket()
{
	const int type = data->protocol == TCP ? SOCK_STREAM : SOCK_DGRAM;
	socket = static_cast<socket_t>(::socket(AF_INET, type, 0));
	if (socket_invalid(socket))
		throw_socket_error("socket");
}

int Socket::getfd() const
{
	return socket;
}

void Socket::set_local_params(addr_t address, long port)
{
	if (address != 0)
		data->local.sin_addr.s_addr = address;
	if (port != 0)
		data->local.sin_port = htons(static_cast<std::uint16_t>(port));
}

void Socket::set_remote_params(addr_t address, long port)
{
	if (address != 0)
		data->remote.sin_addr.s_addr = address;
	if (port != 0)
		data->remote.sin_port = htons(static_cast<std::uint16_t>(port));
}

void Socket::connect(addr_t address, long port, Proto protocol)
{
	set_proto(protocol);
	if (socket_invalid(socket))
		init_socket();
	set_remote_params(address, port);

	if (net_error(::connect(
		    socket,
		    as_sockaddr(data->remote),
		    sizeof(SocketAddress))))
		throw_socket_error("connect");

	socket_length length = sizeof(SocketAddress);
	if (net_error(::getsockname(
		    socket,
		    as_sockaddr(data->local),
		    &length)))
		throw_socket_error("getsockname");
}

void Socket::listen(long port)
{
	set_local_params(0, port);

	const int reuse_address = 1;
	if (net_error(reaper::hw::net::setsockopt(
		    socket,
		    SOL_SOCKET,
		    SO_REUSEADDR,
		    &reuse_address,
		    sizeof(reuse_address))))
		throw_socket_error("setsockopt");

	if (net_error(::bind(
		    socket,
		    as_sockaddr(data->local),
		    sizeof(SocketAddress))))
		throw_socket_error("bind");

	socket_length length = sizeof(SocketAddress);
	if (net_error(::getsockname(
		    socket,
		    as_sockaddr(data->local),
		    &length)))
		throw_socket_error("getsockname");

	if (data->protocol == TCP && net_error(::listen(socket, 10)))
		throw_socket_error("listen");
}

std::unique_ptr<Socket> Socket::accept()
{
	SocketAddress address{};
	socket_length length = sizeof(address);
	const socket_t accepted =
		static_cast<socket_t>(::accept(socket, as_sockaddr(address), &length));
	if (socket_invalid(accepted))
		throw_socket_error("accept");
	return std::unique_ptr<Socket>(new Socket(accepted));
}

addr_t Socket::local_addr() const
{
	return data->local.sin_addr.s_addr;
}

addr_t Socket::remote_addr() const
{
	return data->remote.sin_addr.s_addr;
}

long Socket::remote_port() const
{
	return ntohs(data->remote.sin_port);
}

void Socket::close()
{
	if (socket_invalid(socket))
		return;
	closesocket(socket);
	socket = -1;
	data->reset();
}

int Socket::send(const char* buffer, std::size_t length)
{
	if (data->protocol == TCP)
		return static_cast<int>(::send(socket, buffer, length, 0));

	return static_cast<int>(::sendto(
		socket,
		buffer,
		length,
		0,
		as_sockaddr(data->remote),
		sizeof(SocketAddress)));
}

int Socket::recv(char* buffer, std::size_t length, bool block)
{
	if (data->protocol == TCP)
		return static_cast<int>(::recv(socket, buffer, length, 0));

	socket_length address_length = sizeof(SocketAddress);
#ifdef _WIN32
	if (!block) {
		u_long nonblocking = 1;
		ioctlsocket(socket, FIONBIO, &nonblocking);
		const int result = ::recvfrom(
			socket,
			buffer,
			static_cast<int>(length),
			0,
			as_sockaddr(data->remote),
			&address_length);
		nonblocking = 0;
		ioctlsocket(socket, FIONBIO, &nonblocking);
		return result;
	}
	return ::recvfrom(
		socket,
		buffer,
		static_cast<int>(length),
		0,
		as_sockaddr(data->remote),
		&address_length);
#else
	return static_cast<int>(::recvfrom(
		socket,
		buffer,
		length,
		block ? 0 : MSG_DONTWAIT,
		as_sockaddr(data->remote),
		&address_length));
#endif
}

int Socket::send(
	const char* buffer,
	std::size_t length,
	addr_t address,
	long port)
{
	set_remote_params(address, port);
	return send(buffer, length);
}

int Socket::recv(
	char* buffer,
	std::size_t length,
	addr_t* address,
	long* port,
	bool block)
{
	const int result = recv(buffer, length, block);
	if (address != nullptr)
		*address = data->remote.sin_addr.s_addr;
	if (port != nullptr)
		*port = ntohs(data->remote.sin_port);
	return result;
}

void Socket::set_proto(Proto protocol)
{
	if (protocol == data->protocol)
		return;

	if (!socket_invalid(socket))
		closesocket(socket);
	socket = -1;
	data->protocol = protocol;
	init_socket();
}

std::string addr2string(addr_t address)
{
	in_addr native_address{};
	native_address.s_addr = address;
	std::array<char, INET_ADDRSTRLEN> buffer{};
	const char* result =
		::inet_ntop(AF_INET, &native_address, buffer.data(), buffer.size());
	return result != nullptr ? result : std::string{};
}

bool wait_read(
	const SSeq& input,
	SSeq& readable,
	SSeq& errors,
	int timeout)
{
	fd_set read_set;
	fd_set error_set;
	FD_ZERO(&read_set);
	FD_ZERO(&error_set);

	int max_fd = -1;
	for (Socket* socket : input) {
		const int descriptor = socket->getfd();
		max_fd = std::max(max_fd, descriptor);
		FD_SET(descriptor, &read_set);
		FD_SET(descriptor, &error_set);
	}

	timeval duration{};
	timeval* duration_ptr = nullptr;
	if (timeout >= 0) {
		duration.tv_sec = timeout / 1'000;
		duration.tv_usec = (timeout % 1'000) * 1'000;
		duration_ptr = &duration;
	}

	const int count =
		::select(max_fd + 1, &read_set, nullptr, &error_set, duration_ptr);
	if (net_error(count))
		return false;

	for (Socket* socket : input) {
		const int descriptor = socket->getfd();
		if (FD_ISSET(descriptor, &read_set))
			readable.push_back(socket);
		if (FD_ISSET(descriptor, &error_set))
			errors.push_back(socket);
	}
	return errors.empty();
}

}
