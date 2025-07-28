#ifndef REAPER_MESH_CONTRACTION_H
#define REAPER_MESH_CONTRACTION_H

#include <vector>
#include <set> 

#include "main/types.h"
#include "hw/compat.h"

#include "srmesh.h"
#include "shared.h"
#include "quadrics.h"

namespace reaper 
{
namespace gfx
{
namespace mesh
{

class VFRelations;

/*************************************************************
/                 CLASS: Contraction
/
/  Holds one contraction. Computes the cost at creation and
/  then when an update is demanded via update;
/*************************************************************/
class Contraction {
private:
	Point point;
	float cost;
	const SRMesh* mesh;
	std::set<int> affected;
	const VFRelations* vfrelations;
	bool refused_explicitly;

	float calc_cost(int tmp_vu, int tmp_vt, const Point& parent);
	void set_optimal_point_and_cost(void);
	void verify_optimal_point_and_cost(void);
	void order(void);


public:
	int vt, vu;
	
	int subdued;

	Contraction(const Pair& pair,const SRMesh* srmesh, const VFRelations* vfrel);
	Contraction(const Contraction& arg);

	void update(int s1, int s2, int d);
	void verify_update(int s1, int s2, int d);

	Point get_point(void) const {return point;}

	float get_cost(void) const {return cost;}
	void  set_cost(float f){ cost = f;};

	void inflate(void){cost += 100;}

	const SRMesh* get_mesh(void){return mesh;}

	bool invalid(void);

	void refuse(void){refused_explicitly = true;}
	bool refused(void);

	int estimate_face_collapse(void);

	void report(void);

	bool operator<(Contraction &c1){
		return (cost < c1.cost);
	}

	bool operator==(Contraction &c1){
		return (cost == c1.cost);
	}


};


}
}
}

#endif