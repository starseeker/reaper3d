#include "net/sockstream.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

#include "net/nameservice.h"

namespace reaper::net
{

sock_streambuf::sock_streambuf(const std::string& host, int port)
	: owned_socket(std::make_unique<Socket>()),
	  socket(owned_socket.get())
{
	init_buffers();

	NameData name_data;
	NameService* name_service = NameService::Instance();
	name_service->Lookup(host);
	name_service->Result(true, name_data);
	if (name_data.addrs.empty())
		throw network_error("Name lookup returned no addresses for " + host);
	socket->connect(name_data.addrs.front(), port);
}

sock_streambuf::sock_streambuf(Socket* external_socket)
	: socket(external_socket)
{
	if (socket == nullptr)
		throw std::invalid_argument("sock_stream requires a socket");
	init_buffers();
}

void sock_streambuf::init_buffers()
{
	setg(input_buffer + 4, input_buffer + 4, input_buffer + 4);
	setp(output_buffer, output_buffer + sizeof(output_buffer));
}

int sock_streambuf::read_some()
{
	const int putback_count =
		std::min(static_cast<int>(gptr() - eback()), 4);
	std::memmove(
		input_buffer + (4 - putback_count),
		gptr() - putback_count,
		putback_count);

	const int count =
		socket->recv(input_buffer + 4, sizeof(input_buffer) - 4);
	if (count <= 0) {
		setg(nullptr, nullptr, nullptr);
		return -1;
	}

	setg(
		input_buffer + 4 - putback_count,
		input_buffer + 4,
		input_buffer + 4 + count);
	return count;
}

sock_streambuf::traits_type::int_type sock_streambuf::underflow()
{
	if (gptr() < egptr())
		return traits_type::to_int_type(*gptr());

	if (read_some() < 0)
		return traits_type::eof();
	return traits_type::to_int_type(*gptr());
}

sock_streambuf::traits_type::int_type sock_streambuf::uflow()
{
	const traits_type::int_type character = underflow();
	if (!traits_type::eq_int_type(character, traits_type::eof()))
		gbump(1);
	return character;
}

sock_streambuf::traits_type::int_type sock_streambuf::pbackfail(
	traits_type::int_type character)
{
	if (gptr() == eback())
		return traits_type::eof();

	gbump(-1);
	if (!traits_type::eq_int_type(character, traits_type::eof()))
		*gptr() = traits_type::to_char_type(character);
	return traits_type::not_eof(character);
}

bool sock_streambuf::more() const
{
	return gptr() < egptr();
}

int sock_streambuf::write_some()
{
	const int total = static_cast<int>(pptr() - pbase());
	int sent = 0;
	while (sent < total) {
		const int count =
			socket->send(pbase() + sent, static_cast<std::size_t>(total - sent));
		if (count <= 0) {
			const int remaining = total - sent;
			std::memmove(pbase(), pbase() + sent, remaining);
			setp(output_buffer, output_buffer + sizeof(output_buffer));
			pbump(remaining);
			return -1;
		}
		sent += count;
	}

	setp(output_buffer, output_buffer + sizeof(output_buffer));
	return sent;
}

sock_streambuf::traits_type::int_type sock_streambuf::overflow(
	traits_type::int_type character)
{
	if (write_some() < 0)
		return traits_type::eof();
	if (!traits_type::eq_int_type(character, traits_type::eof()))
		return sputc(traits_type::to_char_type(character));
	return traits_type::not_eof(character);
}

int sock_streambuf::sync()
{
	return write_some() < 0 ? -1 : 0;
}

std::streamsize sock_streambuf::xsgetn(
	traits_type::char_type* destination,
	std::streamsize count)
{
	std::streamsize read = 0;
	while (read < count) {
		const traits_type::int_type character = uflow();
		if (traits_type::eq_int_type(character, traits_type::eof()))
			break;
		*destination++ = traits_type::to_char_type(character);
		++read;
	}
	return read;
}

std::streamsize sock_streambuf::xsputn(
	const traits_type::char_type* source,
	std::streamsize count)
{
	std::streamsize written = 0;
	while (written < count) {
		const auto available = epptr() - pptr();
		if (available == 0 && write_some() < 0)
			break;

		const auto chunk = std::min<std::streamsize>(
			count - written,
			epptr() - pptr());
		std::memcpy(pptr(), source + written, static_cast<std::size_t>(chunk));
		pbump(static_cast<int>(chunk));
		written += chunk;
	}
	return written;
}

sock_stream::sock_stream(const std::string& host, int port)
	: std::iostream(nullptr),
	  socket_buffer(host, port)
{
	init(&socket_buffer);
}

sock_stream::sock_stream(Socket* socket)
	: std::iostream(nullptr),
	  socket_buffer(socket)
{
	init(&socket_buffer);
}

}
