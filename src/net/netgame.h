

#ifndef NET_NETGAME_H
#define NET_NETGAME_H

#include "hw/abstime.h"
#include "hw/reltime.h"
#include "net/net.h"

namespace reaper {
namespace hw {
	namespace event { class EventFilter; }
}
namespace net {

class sock_stream;
class ServerTalk;


struct GameInfo
{
	GameInfo() { }
	PlayerID local_id;
	Players players;
	GameState state;
	hw::time::TimeSpan start_time;
};



class NetGame
{
	sock_stream* conn;
	GameInfo game;
	ServerTalk* srv_talk;
	hw::concurrent::Mutex sync_mtx;
	PlayerID my_id, next_id;
public:
	NetGame();
	~NetGame();

	hw::event::EventFilter* connect(std::string srv);
	void shutdown();

	PlayerID join(bool observer);
	bool start_net_game();

	bool get_multistatus();
	const Players& players() const;
	bool is_connected() const;
//	hw::time::RelTime until_start() const;
	bool sync_start();

	void send_objinfo(const ObjState& st);
	bool get_objinfo(ObjState& st);
};



}
}

#endif

