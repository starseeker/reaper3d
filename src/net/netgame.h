

#ifndef NET_NETGAME_H
#define NET_NETGAME_H

#include <memory>

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
	GameInfo() = default;
	PlayerID local_id = 0;
	Players players;
	GameState state = init;
	hw::time::TimeSpan start_time{};
};



class NetGame
{
	std::unique_ptr<sock_stream> conn;
	GameInfo game;
	std::shared_ptr<ServerTalk> srv_talk;
	hw::concurrent::Mutex sync_mtx;
	PlayerID my_id, next_id;
public:
	NetGame();
	~NetGame();

	NetGame(const NetGame&) = delete;
	NetGame& operator=(const NetGame&) = delete;

	std::unique_ptr<hw::event::EventFilter> connect(std::string srv);
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
