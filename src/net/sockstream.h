#ifndef REAPER_NET_SOCKSTREAM_H
#define REAPER_NET_SOCKSTREAM_H

#include <iostream>
#include <memory>
#include <string>

#include "hw/socket.h"

namespace reaper::net
{

using hw::net::Socket;

class sock_streambuf : public std::basic_streambuf<char> {
	std::unique_ptr<Socket> owned_socket;
	Socket* socket;
	char input_buffer[512];
	char output_buffer[512];

	void init_buffers();
	int read_some();
	int write_some();

public:
	sock_streambuf(const std::string& host, int port);
	explicit sock_streambuf(Socket* socket);
	~sock_streambuf() override = default;

	using traits_type = std::char_traits<char>;

	traits_type::int_type overflow(traits_type::int_type character) override;
	traits_type::int_type underflow() override;
	traits_type::int_type uflow() override;
	traits_type::int_type pbackfail(traits_type::int_type character) override;
	int sync() override;
	std::streamsize xsgetn(
		traits_type::char_type* destination,
		std::streamsize count) override;
	std::streamsize xsputn(
		const traits_type::char_type* source,
		std::streamsize count) override;
	bool more() const;
};

class sock_stream : public std::iostream {
	sock_streambuf socket_buffer;

public:
	sock_stream(const std::string& host, int port);
	explicit sock_stream(Socket* socket);
	bool more() const { return socket_buffer.more(); }
};

}

#endif
