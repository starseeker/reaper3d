/*
 * $Author: pstrand $
 * $Date: 2002/09/13 07:44:57 $
 * $Log: ship_bomber.cpp,v $
 * Revision 1.14  2002/09/13 07:44:57  pstrand
 * gcc-varningar
 *
 * Revision 1.13  2002/05/21 10:03:15  pstrand
 * world-iter.
 *
 * Revision 1.12  2002/01/31 04:55:52  peter
 * no message
 *
 * Revision 1.11  2001/12/03 18:06:15  peter
 * object/base.h-meck...
 *
 * Revision 1.10  2001/11/27 00:54:51  peter
 * worlditeratorer l�mnar inte ifr�n sig d�da objekt l�ngre..
 *
 * Revision 1.9  2001/11/11 01:23:09  peter
 * minnesfixar..
 *
 * Revision 1.8  2001/08/20 16:54:52  peter
 * obj.ptr.
 *
 * Revision 1.7  2001/08/06 12:16:02  peter
 * MegaMerge (se strandy_test-grenen f�r diffar...)
 *
 * Revision 1.6.2.1  2001/08/05 14:01:24  peter
 * objektmeck...
 *
 * Revision 1.6  2001/07/30 23:43:12  macke
 * H�pp, d� var det k�rt.
 *
 * Revision 1.5  2001/07/06 01:47:06  macke
 * Refptrfix/headerfilsst�d/objekt-skapande/mm
 *
 * Revision 1.4  2001/05/13 17:22:24  niklas
 * st�d
 *
 * Revision 1.3  2001/05/12 22:05:29  peter
 * world::Line
 *
 * Revision 1.2  2001/05/12 09:59:41  niklas
 * krockar inte lika ofta...
 *
 * Revision 1.1  2001/05/10 02:02:24  niklas
 * Tv� olika skepp-ai
 *
*/

#include "hw/compat.h"
#include "object/ai.h"
#include "object/controls.h"
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
	enum ShipBomberSpec
	{
		AIM_PRECISION = 20,
		DIST_WAYPOINT_REACHED = 100,
		LOOK_AHEAD = 80,
		LOOK_AHEAD_GROUND = 70,
		MAX_ALTITUDE = 1000,
		VIEW_RANGE = 1000,
		DIST1 = 500,
		DIST2 = 200,
		DIST3 = 60,
		DIST_NEW_ATTACK = 500
	};

	ShipBomber::ShipBomber(const SillyData &d, const Vector &v, controls::Ship &sctrl)
	 : ShipBase(d,v,sctrl)
	{}

	ShipBomber::~ShipBomber()
	{}

	float ShipBomber::navigate(const Vector vel, const Vector dir, controls::Ship& sc)
	{		
		// look for obsticals in the way

		world::Line obj_line(pos, pos+norm(vel)*LOOK_AHEAD);
		world::si_iterator si_it = wr->find_si(obj_line);
		world::dyn_iterator dyn_it = wr->find_dyn(obj_line);

		if(si_it!=wr->end_si() || dyn_it!=wr->end_dyn()){
			sc.yaw = -0.7;
			sc.pitch = 1.0;
			sc.thrust = 1.0;
			return 360.0; // don't fire			
		}

		Vector loc_dir;

		// Y A W

		loc_dir.x = vel.x*dir.x + vel.z*dir.z;
		loc_dir.z = (-vel.z)*dir.x + vel.x*dir.z;

		float a = atan2(loc_dir.z, loc_dir.x);
		
		if(fabs(a)>PI/32)
			sc.yaw = copysign(1.0,a*-1); // big angle, turn fast 
		else if(fabs(a)>PI/64)
			sc.yaw = copysign(0.8,a*-1); // medium angle, turn medium
		else
			sc.yaw = (a / PI)*-1;        // small angle, finetune
		
	
		// P I T C H

		float vel_r = sqrt(pow(vel.x,2) + pow(vel.z,2));
		float dir_r = sqrt(pow(dir.x,2) + pow(dir.z,2));

		loc_dir.x = vel_r*dir_r + vel.y*dir.y;
		loc_dir.y = (-vel.y)*dir_r + vel_r*dir.y;

		float b = atan2(loc_dir.y, loc_dir.x);

		if(b>(PI/2))		
			b = PI - b;	
		else if(b<(-PI/2))
			b = -PI - b;

		// make sure we don't hit the ground
		// or climb to high
		world::tri_iterator tri = wr->find_tri(world::Line(pos,pos+norm(vel)*LOOK_AHEAD_GROUND));
		if(tri!=wr->end_tri()){
			sc.pitch = 0.9;
			sc.thrust = 1.0;
		}
		else if(pos.y > MAX_ALTITUDE)
			sc.pitch = -0.2;
		else
			sc.pitch = b / (PI/2);

		return (fabs(a)+fabs(b))*57.296; 
	}

	void ShipBomber::patrol(void)
	{
		if(!waypoints.empty()){
			
			Point pwp = (*wp_it);   // Position of waypoint
			Vector dwp = pwp - pos; // Direction to waypoint
			
			// Check if waypoint reached
			if(length(dwp) < DIST_WAYPOINT_REACHED){
				++wp_it;
				// if last one, start over from first one
				if(wp_it == waypoints.end())
					wp_it = waypoints.begin();
				return;
			}

			// set default patrol thrust
			sc.thrust = 0.5;
		
			// set yaw and pitch, don't care about the aim
			navigate(vel, dwp, sc);
		}

		// Search for silly enemies,
		// if enemies found attack the closest one

		world::Sphere view_sphere(pos, VIEW_RANGE); 
		world::si_iterator si_it = wr->find_si(view_sphere);
										
		float min_dist = 10000000;
		bool target_found = false;

		for(; si_it != wr->end_si(); ++si_it){
			// if target alive and not a friend and not 'Nature'
			if(((*si_it)->get_company() != data.get_company()) &&
			   ((*si_it)->get_company() != Nature) &&
			   ((*si_it)->get_company() != Village) ){

				float dist = length(Vector((*si_it)->get_pos() - pos));
				if(dist < min_dist){
					min_dist = dist;	
					target_ptr = (*si_it).get_weak_ptr(); // remember this target
					target_found = true;
				}
			}
		}
		
		if(target_found)
			fsm->state_transition(EVENT_ENEMY_DETECTED);
	}

	void ShipBomber::attack(void)
	{		
		// Check if target has been killed
		if(target_ptr->is_dead()){
			fsm->state_transition(EVENT_ENEMY_KILLED);	
			sc.fire = false;
			return;					
		}

		Vector dir_t = target_ptr->get_pos() - pos; // Direction to target
		float dist = length(dir_t);

		// Check if target is out of sight
		if(dist > VIEW_RANGE){
			fsm->state_transition(EVENT_TARGET_LOST);	
			sc.fire = false;
			return;					
		}

		if(dist > DIST1)	 // far away => full speed
			sc.thrust = 1.0;
		else if(dist > DIST2)	 // closer now => slow down
			sc.thrust = 0.5;
		else if(dist > DIST3)	 // near => low speed
			sc.thrust = 0.1;
		else{			 // very close, go back and make a new attack run
			
			// calculate point to evade to
			// TODO: make sure it's not outside the terrain
			ep = pos + norm(Vector(vel.x,0,vel.z))*DIST_NEW_ATTACK;
			ep = Point(ep.x, 50 + max(ep.y, wr->get_altitude(Point2D(ep.x, ep.z))), ep.z);
			fsm->state_transition(EVENT_NEW_ATTACK);	
			new_attack = true;
			sc.fire = false;
			return;			
		}
		
		// set ship controls and remember the aim
		float aim = navigate(vel, dir_t, sc);
		
		// Fire if aim is good
		if(aim < AIM_PRECISION)
			sc.fire = true;
		else
			sc.fire = false;
	}	

	void ShipBomber::evade(void)
	{
		Vector dir_ep = ep - pos;
		if(length(dir_ep) < DIST_WAYPOINT_REACHED){
			if(new_attack){
				fsm->state_transition(EVENT_IN_POSITION);	
				new_attack = false;
				return;
			}
			else{
				fsm->state_transition(EVENT_RETURN);
				return;
			}
		}

		sc.thrust = 1.0;
		navigate(vel, dir_ep, sc);
	}
}
}
}
