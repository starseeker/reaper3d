#ifndef REAPER_NET_GAME_MGR_H
#define REAPER_NET_GAME_MGR_H

#include "hw/event.h"
#include "net/net.h"

#include <set>

namespace reaper {
namespace hw {
namespace gfx {
	class Gfx;
}
}

namespace net
{

using hw::event::PlayerID;

typedef std::set<PlayerID> Players;

struct GameMgr_impl;


/** Game manager. Allocates player id:s used for event-handling
 *  and handles the communication with the game server.
 */

class GameMgr
{
	GameMgr_impl* impl;
public:
	GameMgr(hw::gfx::Gfx&);
	~GameMgr();

	void start_server();
	void record_game(bool);
	void playback_game(const std::string& id = "");

	void init(bool networked, std::string srv, bool split);

	PlayerID alloc_id(bool observer);

	bool start_req();
	void go_go_go();

	void add_sync(NetObjPtr);

	void update();

	bool get_multistatus();
	const Players& players();

	void end();

	void shutdown();


};



}
}


#endif

