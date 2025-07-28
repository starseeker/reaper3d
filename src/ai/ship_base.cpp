/*
 * $Author: peter $
 * $Date: 2001/12/03 18:06:14 $
 * $Log: ship_base.cpp,v $
 * Revision 1.7  2001/12/03 18:06:14  peter
 * object/base.h-meck...
 *
 * Revision 1.6  2001/08/20 16:54:52  peter
 * obj.ptr.
 *
 * Revision 1.5  2001/07/30 23:43:12  macke
 * Häpp, då var det kört.
 *
 * Revision 1.4  2001/07/09 13:33:04  peter
 * gcc-3.0 fixar
 *
 * Revision 1.3  2001/07/06 01:47:05  macke
 * Refptrfix/headerfilsstäd/objekt-skapande/mm
 *
 * Revision 1.2  2001/05/13 17:22:24  niklas
 * städ
 *
 * Revision 1.1  2001/05/10 02:02:24  niklas
 * Två olika skepp-ai
 *
 * Revision 1.29  2001/05/09 19:53:58  niklas
 * max altitude, slipper bli hängande i luften
 *
 * Revision 1.28  2001/05/09 00:29:35  niklas
 * Betydligt bättre skepp-AI. Hastighetsanpassning, bättre terräng-undvikning,
 * lösning på "ringdansproblemet" m.m.
 *
 */

#include "hw/compat.h"
#include "object/ai.h"
#include "object/base_data.h"
#include "world/world.h"
#include <cstdlib>
#include <cfloat>
 
namespace reaper
{
namespace object
{
namespace ai
{
	ShipBase::ShipBase(const SillyData &d, const Vector &v, controls::Ship &sctrl)
	 : wr(world::World::get_ref()), data(d), vel(v), sc(sctrl)
	{
		state[0] = new fsm::State(PATROLING, 1);
		state[0]->add_transition(EVENT_ENEMY_DETECTED, ATTACKING);
		
		state[1] = new fsm::State(ATTACKING, 3);
		state[1]->add_transition(EVENT_TARGET_LOST, PATROLING);
		state[1]->add_transition(EVENT_ENEMY_KILLED, PATROLING);
		state[1]->add_transition(EVENT_NEW_ATTACK, EVADING);
		
		state[2] = new fsm::State(EVADING, 2);
		state[2]->add_transition(EVENT_RETURN, PATROLING);
		state[2]->add_transition(EVENT_IN_POSITION, ATTACKING);
		
		fsm = new fsm::FSM(PATROLING); // start at patroling
		
		for(int i=0; i<3; i++)
			fsm->add_state(state[i]);
		
		wp_it = waypoints.end(); // no waypoints yet
		target_ptr = 0;       // no target yet
		new_attack = false;
	}

	ShipBase::~ShipBase()
	{	
		// FSM destructor also deallocates all states inside
		delete fsm;
	}

	void ShipBase::think()
	{
		pos = data.get_pos();
		
		switch(fsm->get_current_state())
		{
			case PATROLING : patrol(); break;
			case ATTACKING : attack(); break;
			case EVADING   : evade();  break;
			default        : cout << "AI error, unknown state" << endl;
		}
	}
		
	void ShipBase::add_waypoint(const Point& p)
	{	
		// if first one, insert and set current waypoint to it
		// else just push it on the back of the list
		if(waypoints.empty()) {
			waypoints.push_back(Point(p.x, p.y + wr->get_altitude(Point2D(p.x,p.z)), p.z));
			wp_it = waypoints.begin();
		} else
			waypoints.push_back(Point(p.x, p.y + wr->get_altitude(Point2D(p.x,p.z)), p.z));
	}

	void ShipBase::del_waypoint(Point& p)
	{
		waypoints.remove(Point(p.x, p.y + wr->get_altitude(Point2D(p.x,p.z)), p.z));
	}

	void ShipBase::receive(const Message &m)
	{
		messages.push(m);
	}

}
}
}
