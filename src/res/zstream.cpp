/*
 * $Author: pstrand $
 * $Date: 2002/05/04 12:24:50 $
 * $Log: zstream.cpp,v $
 * Revision 1.10  2002/05/04 12:24:50  pstrand
 * *** empty log message ***
 *
 * Revision 1.9  2002/04/06 22:25:32  pstrand
 * win32fixar
 *
 * Revision 1.8  2002/04/06 20:08:25  pstrand
 * *** empty log message ***
 *
 * Revision 1.7  2002/01/21 20:57:40  peter
 * no message
 *
 * Revision 1.6  2001/07/27 15:46:45  peter
 * *** empty log message ***
 *
 * Revision 1.5  2001/07/09 13:33:11  peter
 * gcc-3.0 fixar
 *
 * Revision 1.4  2001/04/23 20:39:01  peter
 * *** empty log message ***
 *
 * Revision 1.3  2001/04/19 01:13:05  peter
 * *** empty log message ***
 *
 * Revision 1.2  2001/04/18 23:55:54  peter
 * zstream... (möjligt det inte funkar helt..)
 *
 * Revision 1.1  2001/04/17 22:54:40  peter
 * zstream
 *
 *
 */

#include "hw/debug.h"
#include "res/zstream.h"
#include "misc/iostream_helper.h"

#include "zlib.h"

#include <array>
#include <iostream>
#include <stdexcept>

namespace reaper
{
namespace res
{


class GZip
{
	z_stream zstr{};
	std::streambuf& is;
	std::array<unsigned char, 4096> read_buf{};
	bool is_eof;
public:
	GZip(std::streambuf& s);
	~GZip();
	int read(unsigned char* ptr, int len);
	bool eof() const;
};

void ignore_until(std::streambuf& is, char stop)
{
	int c;
	while ((c = is.sbumpc()) != stop && c != -1)
	{ }
}

void ignore_bytes(std::streambuf& is, unsigned int count)
{
	while (count-- > 0 && is.sbumpc() != std::char_traits<char>::eof())
		;
}

GZip::GZip(std::streambuf& s) : is(s), is_eof(false)
{
	std::array<unsigned char, 10> hdr{};
	if (is.sgetn(reinterpret_cast<char*>(hdr.data()), hdr.size()) !=
	    static_cast<std::streamsize>(hdr.size()) ||
	    hdr[0] != 0x1f || hdr[1] != 0x8b || hdr[2] != 8) {
		throw std::runtime_error("Invalid gzip header");
	}
	if (hdr[3] & 4) {
		const int low = is.sbumpc();
		const int high = is.sbumpc();
		if (low == std::char_traits<char>::eof() ||
		    high == std::char_traits<char>::eof()) {
			throw std::runtime_error("Truncated gzip extra-field header");
		}
		ignore_bytes(is, static_cast<unsigned int>(low | (high << 8)));
	}
	if (hdr[3] & 8)
		ignore_until(is, '\0');
	if (hdr[3] & 16)
		ignore_until(is, '\0');
	if (hdr[3] & 2) {
		ignore_bytes(is, 2);
	}
	if (inflateInit2(&zstr, -MAX_WBITS) != Z_OK)
		throw std::runtime_error("Unable to initialize gzip decompression");
}

GZip::~GZip()
{
	inflateEnd(&zstr);
}

int GZip::read(unsigned char* ptr, int len)
{
	if (is_eof)
		return -1;
	zstr.next_out = ptr;
	zstr.avail_out = len;
	while (zstr.avail_out > 0) {
		if (zstr.avail_in == 0) {
			const std::streamsize count = is.sgetn(
				reinterpret_cast<char*>(read_buf.data()),
				read_buf.size());
			if (count <= 0) {
				is_eof = true;
				break;
			}
			zstr.next_in = read_buf.data();
			zstr.avail_in = static_cast<uInt>(count);
		}
		const int status = inflate(&zstr, Z_NO_FLUSH);
		if (status == Z_STREAM_END) {
			is_eof = true;
			break;
		}
		if (status != Z_OK && status != Z_BUF_ERROR)
			throw std::runtime_error("Invalid compressed gzip data");
	}
	const int count = len - static_cast<int>(zstr.avail_out);
	return (count == 0 && is_eof) ? -1 : count;
}

bool GZip::eof() const
{
	return is_eof;
}

using ct = std::char_traits<char>;

izstreambuf::izstreambuf(std::streambuf& s)
 : is(s), gzip(std::make_unique<GZip>(is))
{
	setg(ibuf.data(), ibuf.data(), ibuf.data());
}

izstreambuf::~izstreambuf() = default;

int izstreambuf::read_some()
{
	int n = gzip->read(
		reinterpret_cast<unsigned char*>(ibuf.data()),
		static_cast<int>(ibuf.size()));
	if (n == -1) {
		setg(nullptr, nullptr, nullptr);
		return -1;
	} else {
		setg(ibuf.data(), ibuf.data(), ibuf.data() + n);
		return n;
	}
}

ct::int_type izstreambuf::underflow()
{
	if (gptr() < egptr())
		return ct::to_int_type(*gptr());
	
	if (read_some() < 0)
		return ct::eof();
	else
		return ct::to_int_type(*gptr());
}


typedef std::istream std_istream;

izstream::izstream(std::istream& is)
 : std_istream(0)
 , z_buf(*is.rdbuf())
{
	init(&z_buf);
}


}
}
