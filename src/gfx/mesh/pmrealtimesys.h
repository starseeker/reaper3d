#ifndef REAPER_MESH_PMREALTIMESYS_H
#define REAPER_MESH_PMREALTIMESYS_H

#include "shared.h"
#include "srmesh.h"
#include "gfx/camera.h"

namespace reaper 
{
namespace gfx
{
namespace mesh
{

class PMRealTimeSys 
{
private:
	SRMesh& mesh;

	gfx::Camera cam;
	float cam_fov_cos;

	int lower_bound;
	int max_stack_depth;
	float tau;


	Timer ref_timer;

	HashTab qtab;

	void fix_restore(lint::iterator& li, int vi);


public:

	float frame_time;
	int render_cnt;
	int qrefine_cnt;
	int split_cnt;
	int stack_depth;

	//int access[100000];

	void set_max_stack_depth(int param){max_stack_depth = param;};
	void set_lower_bound(int param){lower_bound = param;};


	PMRealTimeSys(SRMesh& srmesh):mesh(srmesh),tau(0.008), render_cnt(0), frame_time(0.0), lower_bound(0){}

	void set_cam(Point origin, Point dir, float fov);

	void refine(void);

	inline bool qrefine(int vi);

	inline bool outside_view_frustum(PMVertex& v);
	inline bool oriented_away(PMVertex& v);
	inline float screen_space_error(PMVertex& v);

	inline bool ecol_legal(PMVertex& v);
	inline bool vsplit_legal(PMVertex& v);

	void force_vsplit(int vi, lint::iterator& li);
	void ecol(int vi, lint::iterator& li);
	void backtrace_ecol(int vi, lint::iterator& li);
	void vsplit(int vi, lint::iterator* li, lint::iterator& tracer);

	void relocate_vertex(int vi, lint::iterator& li);


	int get_face_count(void){return mesh.active_vertices.size();};


	void report(bool verbose);
		
	void render(void);

};

}
}
}

#endif