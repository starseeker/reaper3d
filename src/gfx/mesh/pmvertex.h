#ifndef REAPER_MESH_PMVERTEX_H
#define REAPER_MESH_PMVERTEX_H

#include <list>
#include <vector>
#include "main/types.h"

namespace reaper 
{
namespace gfx
{
namespace mesh
{

class Contraction;

struct RefineInfo {
	float bsr; // Bounding sphere radius
};


class PMSkeleton {
public:
	int vertex_count;
	Point* vertices;
};


class PMVertex {
public:
	std::list<int>::iterator active; // list stringing active vertices V
	bool is_active;
	Point point;
	Vector normal;
	int parent;		// -1 if this vertex is in M0
	int vt;			// -1 if this vertex is in ^M
	int vu;			// -1 if this vertex is in ^M
	
	// Remaining fields encode vsplit information, defined if vt 6= 0.
	int flr[2];				
	int fn[4];				// required neighbours fn0; fn1; fn2; fn3
	int fni[4];				// Indicates which index points to flr
	float flr_area;			// Used for quick-n-dirty screen-space-error calculation
	RefineInfo refine_info;

	PMVertex& operator=(const Contraction& c);

	PMVertex(void){};
	PMVertex(const Contraction& c);		

	bool has_children(void);
	bool has_parent(void);
	
	void report(int i) const;

	void render(void);
};


struct PMFace {
	std::list<int>::iterator active; // list stringing active faces F
	bool is_active;
	int matid;			// material identifier
	Point normal;
	Point color;

	// Remaining fields are used if the face is active.
	int vertices[3];	// ordered counter-clockwise
	int nf[3];	// neighbouring faces[i] across from vertices[i]

	void report(int i) const;
};

}
}
}

#endif