/*
 * $Author: fizzgig $
 * $Date: 2002/05/16 23:58:59 $
 * $Log: misc.cpp,v $
 * Revision 1.36  2002/05/16 23:58:59  fizzgig
 * Preliminary dynamic cubemapping
 *
 * Revision 1.35  2002/04/14 19:27:06  pstrand
 * try again... (uniqueptr)
 *
 * Revision 1.34  2002/04/14 09:22:04  pstrand
 * quadtree redesign and Sphere::radius -> Sphere::r
 *
 * Revision 1.33  2002/04/11 01:03:34  pstrand
 * explicit res_stream exception...
 *
 * Revision 1.32  2002/02/26 22:41:17  pstrand
 * mackefix
 *
 * Revision 1.31  2002/02/08 11:27:53  macke
 * rendering reorg and silhouette shadows!
 *
 * Revision 1.30  2002/01/26 23:27:12  macke
 * Matrox fix, plus some misc stuff..
 *
 * Revision 1.29  2002/01/23 10:17:36  macke
 * hmf!
 *
 */

#include "hw/compat.h"


#include "gfx/misc.h"
#include "gfx/abstracts.h"
#include "gfx/camera.h"
#include "gfx/exceptions.h"
#include "gfx/settings.h"
#include "gfx/displaylist.h"
#include "gfx/managers.h"

#include "hw/gl.h"
#include "hw/debug.h"
#include "world/query_obj.h"
#include "main/types_ops.h"

#include <vector>
#include <cmath>

namespace reaper {
namespace gfx {
namespace misc {
namespace {
reaper::debug::DebugOutput dout("gfx::misc",0);

// Simple sphere rendering without GLU quadrics
void render_sphere_immediate(double radius, int slices, int stacks)
{
	const double PI = 3.14159265358979323846;
	
	glBegin(GL_TRIANGLES);
	
	for (int i = 0; i < stacks; ++i) {
		double lat0 = PI * (-0.5 + (double)i / stacks);
		double z0 = sin(lat0);
		double zr0 = cos(lat0);
		
		double lat1 = PI * (-0.5 + (double)(i + 1) / stacks);
		double z1 = sin(lat1);
		double zr1 = cos(lat1);
		
		for (int j = 0; j < slices; ++j) {
			double lng = 2 * PI * (double)j / slices;
			double x = cos(lng);
			double y = sin(lng);
			
			// First triangle
			glNormal3d(x * zr0, y * zr0, z0);
			glVertex3d(radius * x * zr0, radius * y * zr0, radius * z0);
			
			glNormal3d(x * zr1, y * zr1, z1);
			glVertex3d(radius * x * zr1, radius * y * zr1, radius * z1);
			
			lng = 2 * PI * (double)(j + 1) / slices;
			x = cos(lng);
			y = sin(lng);
			
			glNormal3d(x * zr1, y * zr1, z1);
			glVertex3d(radius * x * zr1, radius * y * zr1, radius * z1);
			
			// Second triangle
			lng = 2 * PI * (double)j / slices;
			x = cos(lng);
			y = sin(lng);
			
			glNormal3d(x * zr0, y * zr0, z0);
			glVertex3d(radius * x * zr0, radius * y * zr0, radius * z0);
			
			lng = 2 * PI * (double)(j + 1) / slices;
			x = cos(lng);
			y = sin(lng);
			
			glNormal3d(x * zr1, y * zr1, z1);
			glVertex3d(radius * x * zr1, radius * y * zr1, radius * z1);
			
			glNormal3d(x * zr0, y * zr0, z0);
			glVertex3d(radius * x * zr0, radius * y * zr0, radius * z0);
		}
	}
	
	glEnd();
}

}

int DisplayList::compilation_in_progress = 0;

//////////////////////////////////////

void meter(float x, float y, float length, float width, float maxval, float value)
{
	float rel_value = (length/maxval)*value;
	rel_value = std::min(rel_value,1.0f);
	rel_value = std::max(rel_value,0.0f);
	glRectf(x, y, x+rel_value, y+width);
}

void sphere(double radius, int slices, int stacks)
{
	render_sphere_immediate(radius, slices, stacks);
}

void sphere(const world::Sphere &s, int slices, int stacks)
{
	glPushMatrix();
	glTranslate(s.p);
	sphere(s.r,slices,stacks);
	glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////


} // end namespace misc

void SimEffect::simulate(float dt)
{
	throw gfx_fatal_error("SimEffect::simulate - not implemented!");
}

void Material::use() const
{
        glMaterial(GL_FRONT,GL_AMBIENT,ambient_color);
        glMaterial(GL_FRONT,GL_DIFFUSE,diffuse_color);
        glMaterial(GL_FRONT,GL_SPECULAR,specular_color);
        glMaterial(GL_FRONT,GL_EMISSION,emission_color);
        glMaterial(GL_FRONT,GL_SHININESS,shininess);
        throw_on_gl_error("Material::use() - exiting");
}

RenderInfo::RenderInfo(const std::string &mesh_file, const Matrix &m, bool b)
: next_ri(0), mtx(&m), param(0), blend(b), radius(0), mesh(mesh_file),
  texture("")
{
	MeshRef mr;

	radius  = mr->get_radius(mesh);
	texture = mr->get_texture(*this);
}

RenderInfo::RenderInfo(const std::string &mesh_file, const Matrix &m, MtxParams &p, bool b)
: next_ri(0), mtx(&m), param(&p), blend(b), radius(0), mesh(mesh_file),
  texture("")
{
	MeshRef mr;

	radius = mr->get_radius(mesh);
	texture = mr->get_texture(*this);
}

void RenderInfo::link(RenderInfo &ri)
{
	if(next_ri) {
		if(ri.next_ri) {
			throw gfx_fatal_error("Illegal attempt to link two renderinfo-lists...");
		}
		ri.next_ri = next_ri;
	}
	next_ri = &ri;
}			

RenderInfo& RenderInfo::operator=(const RenderInfo &ri)
{
	next_ri = ri.next_ri;
	mtx     = ri.mtx;
	param   = ri.param;
	blend   = ri.blend;
	radius  = ri.radius;
	mesh    = ri.mesh;
	texture = ri.texture;
	return *this;
}


}
}
 
