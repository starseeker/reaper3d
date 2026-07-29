/*
 *
 * Gameserver implementation.
 * Maintains one networked game, and except for startup,
 * only distributes events/objectdata. Most of the logic
 * is on the client-side.
 *
 */



/*

 Protocol:

  All communication is client driven, the
  server just responds to requests.

   Requests are oneliners starting with single word (lowercase
   letters only), followed by optional arguments.

   Responses may be empty or span one or several lines,
   format depends on the request.


 request    |    possible response     | comment
 --------------------------------------------------------------------------
 serverinfo    | <none>            | not implemented yet
 gameinfo      | <state>           | see net/net.h, GameState
               | <id>              | <- may repeat
               | end               |
 time	       | <current-time>    |
 join          | <id> | no         | Only granted in <init> or <active>
 obs           | ack | no          | Join as observer
 start         | ack | no          | Only granted in <active> or <startup>
 go            | ack | no          | Sync. mainloop
 quit          | <none>            | Will close connection
 poll          | <event>           |
               | end               |
 event <event> |                   |
 putobj <data> |                   |
 getobjs       | <data>            | <- may repeat
               | end               |

<data> is basically a dump of ObjState in net/net.h

*/



#include <iostream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <set>
#include <map>
#include <vector>

#include "hw/debug.h"
#include "hw/socket.h"
#include "net/nameservice.h"
#include "net/sockstream.h"
#include "net/net.h"
#include "main/types.h"
#include "main/types_io.h"
#include "misc/sequence.h"
#include "misc/stlhelper.h"
#include "misc/iostream_helper.h"
#include "hw/event.h"
#include "hw/abstime.h"
#include "hw/reltime.h"
#include "net/server.h"

#include "object/phys.h"



namespace reaper {
namespace {
        debug::DebugOutput derr("server", 5);
}
namespace net {

using hw::net::addr_t;
using hw::net::Socket;
using hw::net::addr2string;
using hw::event::Event;
using hw::event::PlayerID;
using hw::event::EventSystem;
using hw::event::EventProxy;
using misc::for_each;
using misc::seq;
using misc::send;
using std::deque;
using std::map;
using std::set;
using std::pair;
using std::make_pair;
using std::string;
using std::flush;
using std::getline;


typedef deque<Event> event_queue;


typedef const std::string& CStrRef;


class Client
{
	std::unique_ptr<Socket> socket;
	sock_stream stream_impl;

	event_queue* incoming_events;
	event_queue my_events;
	int id;
public:
	explicit Client(std::unique_ptr<Socket> connection)
	 : socket(std::move(connection)),
	   stream_impl(socket.get()),
	   incoming_events(nullptr),
	   id(0)
	{ }

	void set_incoming_queue(event_queue* eq)
	{
		incoming_events = eq;
	}

	bool push(const Event& ev) {
		if (incoming_events == 0)
			return false;
		if (ev.recv != -1)
			incoming_events->push_back(ev);
		return true;
	}

	void send(const Event& ev) {
		my_events.push_back(ev);
	}

	bool dump()
	{
		while (! my_events.empty()) {
			net::send(stream_impl, my_events.front());
			my_events.pop_front();
		}
		net::send(stream_impl, "end");
		return true;
	}

	int get_id() const { return id; }
	void set_id(int i) { id = i; }

	sock_stream& stream() { return stream_impl; }

	void log(CStrRef msg) {
		derr << "[server] client: " << addr2string(socket->remote_addr())
		     << ": " << socket->remote_port()
		     << ": " << msg << '\n';
	}
};

using misc::seq;
using misc::cseq;
using misc::find;
using misc::apply_to;

void send_ack(Client* c)
{
	send(c->stream(), "ack");
}

typedef std::set<Client*> Clients;

struct Game
{
	int id;
	GameState state;
	hw::time::TimeSpan start_time;
	string desc;
	Clients players, observers;
	event_queue events;

	ObjStateMap obj_states;

	PlayerID next_id;

	Game()
	 : id(1), state(0), desc("Test game"), next_id(1)
	{ }
};

using ClientMap = map<Socket*, std::unique_ptr<Client>>;

using std::max;

template<class T>
PlayerID max_id(T seq)
{
	PlayerID i = 0;
	for (; seq; ++seq) {
		i = max(i, (*seq)->get_id());
	}
	return i;
}

float avg_sync(const ObjStateMap& objs)
{
	float sum = 0;
	statemap_citer c, e = objs.end();
	for (c = objs.begin(); c != e; ++c)
		sum += c->second.sync;
	return objs.empty() ? 0 : sum / objs.size();
}

using Command = std::function<bool(Client*, CStrRef)>;
using CommandMap = map<string, Command>;

class Server
{
	Socket main;
	ClientMap clients;
	deque<Socket*> active;
	CommandMap commands;
	Game game;
	hw::time::TimeSpan start_time;

	hw::time::TimeSpan last_stats;
	int event_count;

	deque<Client*> scheduled_starts;
public:
	Server()
	{
		const auto bind = [this](auto method) {
			return [this, method](Client* client, CStrRef arguments) {
				return (this->*method)(client, arguments);
			};
		};
		commands.emplace("serverinfo", bind(&Server::server_info));
		commands.emplace("gameinfo", bind(&Server::game_info));
		commands.emplace("join", bind(&Server::join));
		commands.emplace("obs", bind(&Server::obs));
		commands.emplace("start", bind(&Server::start));
		commands.emplace("poll", bind(&Server::poll));
		commands.emplace("event", bind(&Server::event));
		commands.emplace("quit", bind(&Server::quit));
		commands.emplace("time", bind(&Server::time));
		commands.emplace("putobj", bind(&Server::putobj));
		commands.emplace("getobjs", bind(&Server::getobjs));
		commands.emplace("go", bind(&Server::go));

		start_time = hw::time::get_time();
		event_count = 0;
	}

	bool putobj(Client* c, CStrRef ln)
	{
//		c->log("putobj");
		ObjState os(read<ObjState>(ln));
		game.obj_states[os.id] = os;
		return true;
	}

	bool getobjs(Client* c, CStrRef)
	{
//		c->log("getobjs");
		misc::Seq<statemap_iter> so(seq(game.obj_states));
		for (; so; ++so)
			send(c->stream(), so->second);
		send(c->stream(), "end");
		return true;
	}

	bool server_info(Client* c, CStrRef)
	{
		c->log("server info");
		c->stream() << "Hostname: " << addr2string(main.local_addr()) << "  "
		   << clients.size() << " client" << (clients.size() == 1 ? "" : "s")
		   << "\r\nend\r\n" << flush;
		return true;
	}

	bool game_info(Client* c, CStrRef)
	{
		c->log("game info");
		send(c->stream(), game.state);
		Clients::iterator p, e = game.players.end();
		for (p = game.players.begin(); p != e; ++p)
			send(c->stream(), (*p)->get_id());
		send(c->stream(), "end");
		return true;
	}

	struct WithPid {
		int pid;
		WithPid(int p) : pid(p) { }
		bool operator()(const Client* c) const {
			return c->get_id() == pid;
		}
	};

	bool join(Client* c, CStrRef)
	{
		c->log("join");
		if (game.state == net::init)
			game.state = net::active;

		if (game.state != net::active) {
		 	send(c->stream(), "no");
			return true;
		}
		PlayerID pid = game.next_id++;
		send(c->stream(), pid);
		c->set_id(pid);
		game.players.insert(c);
		c->log("join ok, id: " + write<int>(pid) +
		       " players: " + write<int>(game.players.size()));
		return true;
	}

	bool obs(Client* c, CStrRef)
	{
		c->log("obs");
		if (game.state != net::active) {
			send(c->stream(), "no");
			return true;
		}
		game.observers.insert(c);
		send(c->stream(), "ack");
		return true;
	}

	bool start(Client* c, CStrRef)
	{
		c->log("start");

		if (game.state == net::active) {
			game.state = net::startup;
			game.start_time = hw::time::get_time() + hw::time::TimeSpan(0, 2000000);
		}
		if (game.state != net::startup) {
			send(c->stream(), "no");
			game.players.erase(c);
			game.observers.erase(c);
			return false;
		}
		send(c->stream(), "ack");
		c->log("start granted for: " + write<int>(c->get_id()));
		return true;
	}

	bool go(Client* c, CStrRef)
	{
		c->log("go");
		if (game.state != net::startup) {
			send(c->stream(), "no");
			game.players.erase(c);
			game.observers.erase(c);
			return false;
		}
		scheduled_starts.push_back(c);
		c->set_incoming_queue(&game.events);
		c->log("synchronizing start for: " + write<int>(c->get_id()));
		return true;
	}

	bool poll(Client* c, CStrRef)
	{
		return c->dump();
	}

	bool event(Client* c, CStrRef str)
	{
		return c->push(read<Event>(str));
	}

	bool quit(Client* c, CStrRef)
	{
		c->log("quit");
		return false;
	}

	bool time(Client* c, CStrRef)
	{
		hw::time::TimeSpan now = hw::time::get_time();
		c->log("time " + write<hw::time::TimeSpan>(now));
		send(c->stream(), now);
		return true;
	}

	int serve(Socket* s)
	{
		if (s == &main) {
			auto connection = main.accept();
			Socket* socket = connection.get();
			auto client = std::make_unique<Client>(std::move(connection));
			Client* c = client.get();
			clients.emplace(socket, std::move(client));
			active.push_back(socket);
			send(c->stream(), "Welcome");
			c->log("connect");
		} else {
			auto client_entry = clients.find(s);
			if (client_entry == clients.end())
				return 0;
			Client* c = client_entry->second.get();
			do {
				if (!c->stream().good())
					c->log("[server] stream bad!");
				string cmd;
				misc::until(c->stream(), cmd, " \n\r");
				string rest;
				misc::until(c->stream(), rest, "\n\r");
				misc::skip_crlf(c->stream());

				bool client_ok = false;
				auto i = commands.find(cmd);
				if (i == commands.end()) {
					c->log("invalid command: " + cmd);
				} else {
					if (i->second(c, rest) && c->stream().good())
						client_ok = true;
				}
				if (!client_ok) {
					c->log("removing => shutdown");
					s->close();
					active.erase(
						std::remove(active.begin(), active.end(), s),
						active.end());
					game.players.erase(c);
					game.observers.erase(c);
					scheduled_starts.erase(
						std::remove(
							scheduled_starts.begin(),
							scheduled_starts.end(),
							c),
						scheduled_starts.end());
					clients.erase(client_entry);

					if (clients.empty()) {
						game.state = init;
						game.events.clear();
						game.obj_states.clear();
					}
					break;
				}
			} while (c->stream().more());
		}
		return 0;
	}

	void print_stats()
	{
		char buf[200];
		snprintf(buf, 200, "[server] cl: %zu  ac: %zu  ev: %d  tm: %f  rt: %ld  ob: %zu\n",
		     clients.size(),
		     active.size(),
		     event_count,
		     hw::time::get_time().approx().to_s(),
		     hw::time::get_rel_time(),
		     game.obj_states.size());
		derr << buf;
	}

	void do_stuff()
	{
		hw::time::TimeSpan now = hw::time::get_time();

		if (game.state == net::startup &&
		    scheduled_starts.size() == (game.players.size() + game.observers.size())) {
			for_each(seq(scheduled_starts), send_ack);
			game.state = net::running;
		}

		while (! game.events.empty()) {
			++event_count;
			const Event& ev = game.events.front();
			for (auto c = clients.begin(); c != clients.end(); ++c) {
				if (c->second->get_id() != ev.recv)
					c->second->send(ev);
			}
			game.events.pop_front();
		}
		if ((now - last_stats).approx() > 2e6) {
			print_stats();
			last_stats = now;
		}
	}

	void run()
	{
		main.listen(4247);

		sock_stream ms(&main);
		active.push_back(&main);

		bool quit = false;
		derr << "[server] startup\n";
		while (! quit) {
			deque<Socket*> ready, err;
			hw::net::wait_read(active, ready, err, 1000);
			for_each(seq(ready), apply_to(this, &Server::serve));
			do_stuff();
		}
	}

};


GameServer::GameServer()
	: server(std::make_unique<Server>())
{
}

GameServer::~GameServer() = default;

void GameServer::run()
{
	server->run();
}


}
}
