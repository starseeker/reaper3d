#include "hw/compat.h"

#include "res/res.h"
#include "gfx/mesh.h"
#include "gfx/io.h"
#include "hw/debug.h"
#include "main/types_ops.h"
#include "misc/parse.h"

#include <list>

using namespace std;

namespace reaper {
namespace gfx {
namespace mesh {
namespace {

// dummy data for parsing
char buf[256];
string s;

namespace rpmisc = reaper::misc;

void read_geometry(istream &is, vector<Point> &points, vector<Vector> &normals,
		   vector<TexCoord> &texcoords, vector<IdxTriangle> &triangles)
{
        int num_verts;
        is.ignore(256,':') >> num_verts;	// "Vertices:" + "Number of vertices:"

        points.reserve(num_verts);
        normals.reserve(num_verts);
        texcoords.reserve(num_verts);

        read_data<Point>(is,back_inserter(points),"read_geometry() pts ");    
	rpmisc::skipline(is);

        int num_tris;
	skip_label(is) >> num_tris;     // "Number of polygons:"
	rpmisc::skipline(is);
	rpmisc::skipline(is);           // "Polygon connection list:"

        triangles.reserve(num_tris);
        read_data<IdxTriangle>(is,back_inserter(triangles),"read_geometry() tri "); 

        is.getline(buf,256).getline(buf,256);    // "Per vertex normals:"
        read_data<Vector>(is,back_inserter(normals),"read_geometry() norm ");     

        is.getline(buf,256).getline(buf,256);    // "Texture Coordinates:"
        read_data<TexCoord>(is,back_inserter(texcoords),"read_geometry() tex ");                   
        
        if(points.size() != num_verts)
                throw gfx_fatal_error("read_geometry() - Incorrect amount of points read!");
	if(normals.size() != num_verts)
                throw gfx_fatal_error("read_geometry() - Incorrect amount of normals read!");
	if(texcoords.size() != num_verts)
                throw gfx_fatal_error("read_geometry() - Incorrect amount of texcoords read!");
	if(triangles.size() != num_tris)
                throw gfx_fatal_error("read_geometry() - Incorrect amount of triangles read!");
}

void read_texture_id(istream &is, string &texture)
{
        skip_label(is);
	string tex_name;
	getline(is, tex_name);
	int ws_count = tex_name.find_first_not_of(" \r\n");
	if (ws_count != string::npos) {
		tex_name.erase(0, ws_count);
		int tex_end = tex_name.find_first_of(" \r\n");
		if (tex_end != string::npos)
			tex_name.erase(tex_end);
		texture = tex_name;
	}
}

void read_material(istream &is, Material &material)
{
        is.getline(buf,256);    // "Shading data:"
        skip_label(is) >> material.diffuse_color;
        skip_label(is) >> material.ambient_color;
        skip_label(is) >> material.emission_color;
        skip_label(is) >> material.specular_color;
        skip_label(is) >> material.shininess;
        throw_if_bad(is,"MeshFile:  Unable to read material data");
	material.ambient_color = Color(1,1,1);
}

template<typename T>
void read_sub_names(istream &is, T &cont)
{
        is.ignore(256,'[') >> s;

        while(s.find(']') == string::npos) {
                cont.push_back(s);
                is >> s;                       
        }
        is.getline(buf,256);
        throw_if_bad(is,"MeshFile:  Unable to read sub-object names");
}

//FIXME: Remove this stupid hack
void skip_lod_and_animation(istream &is)
{
        // Read LOD  (just ignore this...)
        int lod;
	skip_label(is) >> lod;
        lod = 0;

        // Skip animation data for now
        is.getline(buf,256);
        is.getline(buf,256);
        is >> s >> s >> s;
        throw_if_bad(is,"MeshFile:  Unable to read animation data");
}

void read_transform(istream &is, Matrix &m)
{
	Point pos;
	Quaternion dir;
	Vector scale;

	skip_label(is) >> pos;
	skip_label(is) >> dir;
	skip_label(is) >> scale;
	throw_if_bad(is,"MeshFile:  Unable to read pos/dir/scale");
	
	m = Matrix(pos) * Matrix(dir) * Matrix(scale.x,scale.y,scale.z);
}

void pretransform(Matrix &mtx, vector<Point> &points, vector<Vector> &normals)
{
	for(int i = 0; i<points.size(); i++) {
		points[i] = mtx * points[i];
		normals[i] = mtx * normals[i];
		normd(normals[i]);
	}
}

float calc_bound_radius(vector<Point> &points, const Matrix& mtx = Matrix::id())
{
	float r = 0;
	for(int i = 0; i<points.size(); i++) {
		float vr = length(mtx * points[i]);
		if(vr > r)
			r = vr;
	}
	return r;
}

reaper::debug::DebugOutput dout("gfx::MeshFileRead",0);

} // end anonymous namespace

template <typename T>
void SubObjReader<T>::read_it(const string &name, T &mesh, list<string> &sub_names, Matrix &mtx)
{
        for(list<string>::const_iterator i = sub_names.begin(); i != sub_names.end(); ++i) {
		T sub_obj;
		sub_obj.name = name + '_' + *i;
		sub_obj.read(mtx);
		if(sub_obj.bnd_sph_radius > mesh.bnd_sph_radius) {
			mesh.bnd_sph_radius = sub_obj.bnd_sph_radius;
		}
		mesh.n_vertices  += sub_obj.n_vertices;
		mesh.n_triangles += sub_obj.n_triangles;
		mesh.sub_objs.push_back(sub_obj);
	}
}

void PlainMesh::read(Matrix parent_mtx)
{
        using namespace std;
        
        res::res_stream is(res::Object,name);		

	std::list<string> sub_names;
	Matrix mtx;
	{
	        is.getline(buf,256);
		read_transform(is, mtx);
		skip_lod_and_animation(is);
		read_sub_names(is, sub_names);
		read_material(is, material);
		read_texture_id(is, texture);

		vector<Point> points;		
		vector<TexCoord> texcoords;     
		vector<Vector> normals;         
		vector<IdxTriangle> triangles;  

		read_geometry(is, points, normals, texcoords, triangles);

		mtx = parent_mtx * mtx;
		pretransform(mtx, points, normals);
		bnd_sph_radius = calc_bound_radius(points);
		n_vertices = triangles.size() * 3;
		n_triangles = triangles.size();

		render_gmt.init(&points[0], 0, &normals[0], &texcoords[0], true,
		          points.size(), &triangles[0].v1, n_vertices);	
		geometry.init(points, triangles);
	}

	SubObjReader<PlainMesh>().read_it(is.id(), *this, sub_names, mtx);
}

void ParametricMesh::read(Matrix parent_mtx)
{
	res::res_stream is(res::Object,name);		

	std::list<string> sub_names;
	{
	        is.getline(buf,256);
		read_transform(is, mtx);
		skip_label(is) >> param;
		throw_if_bad(is,"ParametricMesh:  Unable to read parameter info");

		skip_lod_and_animation(is);
		read_sub_names(is, sub_names);
		read_material(is, material);
		read_texture_id(is, texture);

		vector<Point> points;		
		vector<TexCoord> texcoords;     
		vector<Vector> normals;         
		vector<IdxTriangle> triangles;  

		read_geometry(is, points, normals, texcoords, triangles);

		if(param == -1) {
			mtx = parent_mtx * mtx;
			pretransform(mtx, points, normals);
		}
		bnd_sph_radius = calc_bound_radius(points, parent_mtx);
		n_vertices = triangles.size() * 3;
		n_triangles = triangles.size();

		render_gmt.init(&points[0], 0, &normals[0], &texcoords[0], true,
		          points.size(), &triangles[0].v1, n_vertices);	
	}

	SubObjReader<ParametricMesh>().read_it(is.id(), *this, sub_names, mtx);
}

}
}
}

/*
 * $Author: pstrand $
 * $Date: 2002/07/13 14:42:48 $
 * $Log: mesh_file_read.cpp,v $
 * Revision 1.6  2002/07/13 14:42:48  pstrand
 * minifix...
 *
 * Revision 1.5  2002/06/18 12:18:58  pstrand
 * types_io.h upprensning
 *
 * Revision 1.4  2002/05/22 23:29:10  fizzgig
 * enabled shadow volume geometry computation again.. (not used in game currently..will be later..)
 *
 * Revision 1.3  2002/05/22 10:04:46  fizzgig
 * no message
 *
 * Revision 1.2  2002/05/21 03:07:51  fizzgig
 * envmap update options (gfx_renderer::environment_mapping: 0->off 1->incremental 2->instant)
 * preliminary support for shadow volumes (now requests stencil buffer as well)
 *
 * Revision 1.1  2002/04/23 22:50:19  fizzgig
 * mesh-loading cleanup
 *
 * Revision 1.9  2002/04/07 00:05:35  pstrand
 * fix. for old gcc:s
 *
 * Revision 1.8  2002/02/08 11:27:52  macke
 * rendering reorg and silhouette shadows!
 *
 */

