#ifndef REAPER_MESH_PMFACTORY_H
#define REAPER_MESH_PMFACTORY_H

#include "shared.h"
#include "pairs.h"
#include "contractions.h"
#include "main/types.h"

namespace reaper 
{
namespace gfx
{
namespace mesh
{

class PMFactory
{
private:

	Timer timer;

	int subdue_threshold;		// Determines how many passes a contraction can go without rejection

	Pairs pairs;				// Initially generated pairs
	Contractions* contractions;	// Holds unused contractions
	VFRelations* vfrelations;	// Stores vertex-faces relations
	SRMesh* mesh;				// The srmesh to be passed to the real-time system

	void generate_pairs(void);			// Generate initial pairs
	void initiate_contractions(void);	// Make contractions out of pairs
	void pop_and_update(void);			// Add the top contraction to mesh
	void after_pass(void);				// Record the fn configuration
	ContStatus new_pmvertex(const Contraction& cont);// Add a vertex to mesh

	ContStatus initiate_fn(PMVertex& pmv);

	Timer build_timer;
	Timer initfn_timer;
	Timer sort_timer;
	Timer quad_timer;
	Timer update_timer;
	Timer subdue_timer;
	Timer np_timer;
	Timer pop_timer;
	Timer fn_timer;

	bool virgin;
	int	non_prog_limit;

	int lower_bound;

public:
	PMFactory(SRMesh* m);
	~PMFactory(void){};

	PMSkeleton* export_skeleton(void);

	void new_vertex(Point& p);
	void new_face(int a, int b, int c);
	void build(void);	
	void set_lower_bound(int val){lower_bound = val;};

	SRMesh* get_mesh(void){return mesh;};

	void report(bool verbose);
	void report_faces(void);
	void report_active_configuration(void);

	void render(void);

};

}
}
}

#endif