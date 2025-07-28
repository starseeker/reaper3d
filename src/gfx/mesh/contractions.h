#ifndef REAPER_MESH_CONTRACTIONS_H
#define REAPER_MESH_CONTRACTIONS_H

#include <vector>
#include <list>
#include <set> 

#include "main/types.h"
#include "hw/compat.h"

#include "srmesh.h"
#include "pmvertex.h"
#include "contraction.h"
#include "contractionref.h"
#include "vfrelations.h"

namespace reaper 
{
namespace gfx
{
namespace mesh
{




/*************************************************************
/                 CLASS: Contractions
/
/	Holds all potential contractions. Used during the 
/	Factorization.
/*************************************************************/
class Contractions {

private:


	const SRMesh& mesh;
	VFRelations& vfrelations;
	std::vector<ContractionRef> conts;

	std::list<Contraction> ref_list;

	//void find_affected_vertices(int vt, int vu, std::set<int>& target);
	void find_affected_vertices(PMVertex& pmv, std::set<int>& target);
	void potential_update(int index, std::set<int> &affected, PMVertex& pmv, int pmvi);
	void verify_potential_update(int index, std::set<int> &affected, PMVertex& pmv, int pmvi);
	
public:

	float update_time;

	void Contractions::render(void);

	Contractions(const SRMesh& srmesh, VFRelations& vfr);
	Contractions(const Contraction& arg);
	~Contractions(void){};

	void insert(Contraction* new_cont);

	void verify_update(PMVertex& update_pmv);

	void update(PMVertex& update_pmv);
	void fast_update(PMVertex& update_pmv);
	void Contractions::fast_update(Contraction& cont);
	void cascade_cont_update(int vid, PMVertex& pmv);

	void re_sort(void);
	Contraction get_top(void);
	void pop_top(void);
	void subdue_top(void);
	void remove_top(void);

	void clear(void);
	bool empty(void);
	int size(void){return conts.size();};

	void report(bool verbose);
	void verify_report(bool verbose);
	void verify_integrity(void);

};

}
}
}

#endif