#ifndef REAPER_MESH_SRMESH_H
#define REAPER_MESH_SRMESH_H

#include <vector>
#include "main/types.h"
#include "pmvertex.h"
#include "shared.h"
//#include "pmstorage.h"

namespace reaper 
{
namespace gfx
{
namespace mesh
{


class SRMesh {						// Selectively refinable mesh
private:
	int write_file(char* file_name, bool replace);
	void read_file(char* file_name);

public:

	SRMesh(void);

	std::vector<PMVertex> vertices;	// set V of all vertices
	std::vector<PMFace> faces;		// set ^F of all faces
	std::list<int> active_vertices;
	std::list<int> active_faces;

	void add_active_vertex(int vind);
	void add_active_vertex_at(int vind, std::list<int>::iterator dst);
	void add_active_face(int find);
	void remove_active_vertex(int vind);
	void remove_active_face(int find);

	void load(char* file_name);
	void save(char* file_name);

	// Can these be effectively hidden somewhere else?
	void switch_face_neighbour(int src, int old_dest, int new_dest){ switch_face_neighbour(src, old_dest, new_dest, "no caller");};
	void switch_face_neighbour(int src, int old_dest, int new_dest, char* caller);

	void set_face_neighbour(int src, int matching, int new_dest);

	int get_active_vertex_relative(int i) const;
	int get_face_vertex(int fid, int vid) const;

	bool vertex_spawned(int vid);

	void split(int vind);
	void split_children_at(int vind, lint::iterator& dst);

	void col(int vind);
	void col_parent_at(int vind, lint::iterator& dst);

	void render(void);
	void render(PMVertex* pmv);

};

}
}
}

#endif