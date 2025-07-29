/*
 * $Author: peter $
 * $Date: 2001/03/04 14:24:36 $
 * $Log: client.cpp,v $
 * Revision 1.3  2001/03/04 14:24:36  peter
 * gfx config
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

#include <string>
#include <vector>
#include <iostream>

#include "hw/compat.h"

#include "hw/socket.h"
#include "net/nameservice.h"
#include "hw/fdstream.h"
#include "net/sockstream.h"
#include "hw/hwtime.h"

using namespace reaper::hw;
using namespace reaper::net;
using reaper::hw::time::get_time;

int main(int argc, char *argv[]) {
	NameData nd;
	std::vector<int> times;
	Socket s;
	NameService::Instance()->Lookup(argc == 2 ? argv[1] : "localhost");
	NameService::Instance()->Result(true, nd);
	char buf[512];
	long t1, t2;
	strcpy(buf, "hello, world");
	try {
		addr_t a = nd.addrs.front();
		s.connect(a, 4242);
		sock_stream ss(&s);
		ss << "Hello world\n" << std::flush;
		ss >> buf;
		std::cout << "->" << buf << "<-\n";
	} catch (socket_error& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}

