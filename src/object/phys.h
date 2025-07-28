#ifndef REAPER_OBJECT_PHYS_H
#define REAPER_OBJECT_PHYS_H

#include "object/base.h"
#include "object/base_data.h"
#include "object/phys_data.h"
#include "object/forward.h"
#include "phys/engine.h"

namespace reaper {

class MtxParams;

namespace phys {
	class Engine;
	typedef reaper::misc::UniquePtr<reaper::phys::Engine> PhysRef;
}
namespace object {
namespace phys {

/// Used to access physics data for object
class ObjectAccessor
{
	SillyData& sd;
public:
	// A point& pos will be available once we change the
	// matrix-class

	/// Position/Rotation/Translation matrix
	const Matrix& get_mtx() const;
	void set_mtx(const Matrix& m);

	/// Velocity
	Vector &vel;
	/// Rotational velocity
	Vector &rot_vel;
	/// Maximum acceleration
	const float max_acc;
        ///Maximum velocity
        const float max_vel;
	/// Object mass
	const float mass;
	/// Bounding sphere radius 
	const float radius;

	const float damage;
	
	ObjectAccessor(SillyData &sd, DynamicData &dd,
		       const ShipInfo &i, const float &r);

	ObjectAccessor(SillyData &sd, DynamicData &dd,
		       const ShotInfo &i, const float &r);

	ObjectAccessor(SillyData &sd, DynamicData &dd,
		       float ma, float m, float d, float r);
};

class Turret
{
	const controls::Turret &tc;
	MtxParams &pr;
	current_data::Turret& current;
//        weapon::Turret &weap;
        const TurretInfo &info;
	float max_yaw;
	float max_pitch;
	double& sim_time;
	reaper::phys::PhysRef phys_ref;
public:
	Turret(const controls::Turret &t,MtxParams &p, 
		current_data::Turret &current,
//                weapon::Turret &weap,
		const TurretInfo &info,
		double& sim_time);
	
	void simulate(double dt);
};

class internal_rudders
{
public:
	float pitch;
	float yaw;
	float skew;
	internal_rudders() :
	pitch(0),yaw(0),skew(0) {}
};

class Ship
{
	SillyData& data;
        const controls::Ship &tc;
	const ShipInfo &info;
	current_data::Ship &current;
	DynamicData &dyn_data;
//        weapon::LaserShip &weap;
//	weapon::MissileShip &weap_missile;
	double& sim_time;
	internal_rudders rudders;
	short last_ortho;
	reaper::phys::PhysRef phys_ref;
public:
	Ship(SillyData& data,
	     const controls::Ship &tc,
	     current_data::Ship &current,
	     const ShipInfo &info,
	     DynamicData &dyn_data,
//             weapon::LaserShip &weap,
//	     weapon::MissileShip& weap_missile,
	     double& sim_time);

	void simulate(double dt);
        Vector compute_acc(const Vector v);
	float get_afterburner() const;
};

class GroundShip
{
protected:
	SillyData& data;
        const controls::GroundShip &tc;
	const GroundShipInfo &info;
	current_data::GroundShip &current;
	DynamicData &dyn_data;
//                weapon::GroundShip &weap;
	double& sim_time;
	short last_ortho;
        
public:
	GroundShip(SillyData& d,
	     const controls::GroundShip &tc,
	     current_data::GroundShip &current,
	     const GroundShipInfo &info,
	     DynamicData &dyn_data,
//                    weapon::GroundShip &weap,
	     double& sim_time) : 
		data(d),
		tc(tc),
		info(info),
		current(current),
		dyn_data(dyn_data),
//                       weap(weap),
		sim_time(sim_time),
		last_ortho(0)
		{}

	 void simulate(const double dt);
         Vector compute_acc(const Vector v,const double& dt);
};

/*	class GroundTurret : public GroundShip
{
	const controls::Turret &tc;
	MtxParams &pr;
	current_data::Turret& current;
        weapon::Turret &weap;
        const TurretInfo &info;
	float max_yaw;
	float max_pitch;
	double& sim_time;
	reaper::phys::PhysRef phys_ref;
public:
	GroundTurret(const controls::Turret &t,MtxParams &p, 
		current_data::Turret &current,
                weapon::Turret &weap,
		const TurretInfo &info,
		double& sim_time);
public:
	void simulate(const double dt);
};
*/

class Laser
{
        SillyData &d;
        const Vector velocity;
	const ShotInfo& info;
	double& sim_time;
	const double death_time;
public:
        Laser(SillyData &dta,
                const Vector vel,
		const ShotInfo &info,
                double &sim_time,const double death_time) :
        d(dta),velocity(vel),info(info),sim_time(sim_time),death_time(death_time) {}

        void simulate(double dt);
};




// gfx test
class DummyTurret
{
	const controls::Turret &tc;
	MtxParams &pr;
public:
	DummyTurret(const controls::Turret &t,MtxParams &p) : tc(t), pr(p) {}		
	void simulate(double dt);
};

}
}
}
#endif

/*
 * $Author: pstrand $
 * $Date: 2002/04/06 20:13:31 $
 * $Log: phys.h,v $
 * Revision 1.41  2002/04/06 20:13:31  pstrand
 * the matrix encapsulated one more level, use SillyData::set_mtx to change (or objectaccessor)
 *
 * Revision 1.40  2002/02/08 11:27:57  macke
 * rendering reorg and silhouette shadows!
 *
 * Revision 1.39  2001/12/03 18:06:18  peter
 * object/base.h-meck...
 *
 * Revision 1.38  2001/11/10 11:58:34  peter
 * diverse minnesfixar och strul...
 *
 * Revision 1.37  2001/08/06 12:16:38  peter
 * MegaMerge (se strandy_test-grenen för diffar...)
 *
 * Revision 1.36.2.3  2001/08/05 14:01:32  peter
 * objektmeck...
 *
 * Revision 1.36.2.2  2001/08/04 16:14:17  peter
 * *** empty log message ***
 *
 * Revision 1.36.2.1  2001/08/03 13:44:12  peter
 * pragma once obsolete...
 *
 * Revision 1.36  2001/07/30 23:43:29  macke
 * Häpp, då var det kört.
 *
 * Revision 1.35  2001/07/06 01:47:32  macke
 * Refptrfix/headerfilsstäd/objekt-skapande/mm
 *
 * Revision 1.34  2001/05/14 21:37:44  niklas
 * Fungerande ground-vehicle turrets, kanske....
 *
 * Revision 1.33  2001/05/14 11:22:41  picon
 * Fullösningsförberedelse
 *
 * Revision 1.32  2001/05/10 11:40:23  macke
 * häpp
 *
 * Revision 1.31  2001/05/09 23:03:27  picon
 * missiler...
 *
 * Revision 1.30  2001/05/05 06:58:46  peter
 * fix
 *
 * Revision 1.29  2001/05/05 03:18:33  macke
 * Massa bök: uppdaterad renderingsmetod, snyggare explosioner mm
 *
 * Revision 1.28  2001/04/29 19:59:48  picon
 * Adderar en ground vehicle
 *
 */

