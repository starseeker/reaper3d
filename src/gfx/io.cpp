/*
 * $Author: pstrand $
 * $Date: 2002/04/11 01:03:33 $
 * $Log: io.cpp,v $
 * Revision 1.5  2002/04/11 01:03:33  pstrand
 * explicit res_stream exception...
 *
 * Revision 1.4  2001/06/09 01:58:48  macke
 * Grafikmotor reorg
 *
 * Revision 1.3  2001/05/10 11:40:12  macke
 * häpp
 *
 */

#include "hw/compat.h"

#include <vector>
#include <iterator>

#include "main/types.h"
#include "main/types_io.h"
#include "res/res.h"
#include "gfx/io.h"

using namespace std;

namespace reaper {
namespace gfx {
namespace lowlevel {

void read_terrain(const std::string &file, vector<Point> &points, vector<TexCoord> &texcoords,
		  vector<Vector> &normals, vector<Color> &colors, vector<IdxTriangle> &triangles)
{
	using std::back_inserter;               
	res::res_stream rs(res::Terrain,file);

	rs.ignore(256,':');
	rs.ignore(256,':');

	int num_verts;
	rs >> num_verts;

	points.reserve(num_verts);		
	texcoords.reserve(num_verts);     
	normals.reserve(num_verts);         
	colors.reserve(num_verts);

	read_data<Point>(rs,back_inserter(points),"gfx::read_terrain()");
	
	rs.ignore(256,':');

	int num_tris;
	rs >> num_tris;
	triangles.reserve(num_tris);

	rs.ignore(256,':');
	read_data<IdxTriangle>(rs,back_inserter(triangles),"gfx::read_terrain()");
	rs.ignore(256,':');

	read_data<Vector>(rs,back_inserter(normals),"gfx::read_terrain()");
	rs.ignore(256,':');

	read_data<TexCoord>(rs,back_inserter(texcoords),"gfx::read_terrain()");
	rs.ignore(256,':');

	read_data<Color>(rs,back_inserter(colors),"gfx::read_terrain()");

	if(points.size() != num_verts)
		throw gfx_fatal_error("gfx::read_terrain() - Incorrect amount of points read!");
	if(normals.size() != num_verts)
		throw gfx_fatal_error("gfx::read_terrain() - Incorrect amount of normals read!");
	if(texcoords.size() != num_verts)
		throw gfx_fatal_error("gfx::read_terrain() - Incorrect amount of texcoords read!");
	if(colors.size() != num_verts)
		throw gfx_fatal_error("gfx::read_terrain() - Incorrect amount of colors read!");
	if(triangles.size() != num_tris)
		throw gfx_fatal_error("gfx::read_terrain() - Incorrect amount of triangles read!");
}

}
}
}
