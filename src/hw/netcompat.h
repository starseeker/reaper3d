#ifndef REAPER_HW_NETCOMPAT_H
#define REAPER_HW_NETCOMPAT_H

#ifdef _WIN32
#include "hw/windows.h"
#else
#include <arpa/inet.h>
#include <cerrno>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "hw/socket.h"

namespace reaper::hw::net
{

std::string dnsstrerror();
std::string sockstrerror(int error = 0);

#ifdef _WIN32

using socket_length = int;

inline bool net_error(int result) noexcept
{
	return result == SOCKET_ERROR;
}

inline bool socket_invalid(socket_t socket) noexcept
{
	return socket == static_cast<socket_t>(INVALID_SOCKET);
}

inline int setsockopt(
	socket_t socket,
	int level,
	int option,
	const int* value,
	socket_length length)
{
	return ::setsockopt(
		socket,
		level,
		option,
		reinterpret_cast<const char*>(value),
		length);
}

#else

using socket_length = ::socklen_t;

inline bool net_error(int result) noexcept
{
	return result < 0;
}

inline bool socket_invalid(socket_t socket) noexcept
{
	return socket < 0;
}

inline void closesocket(socket_t socket) noexcept
{
	::close(socket);
}

inline int setsockopt(
	socket_t socket,
	int level,
	int option,
	const int* value,
	socket_length length)
{
	return ::setsockopt(socket, level, option, value, length);
}

#endif

}

#endif
