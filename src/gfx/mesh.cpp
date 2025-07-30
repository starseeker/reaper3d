#include "hw/compat.h"

#include "gfx/mesh.h"
#include "gfx/exceptions.h"
#include "gfx/io.h"
#include "gfx/pm/pm.h"
#include "hw/debug.h"
#include "hw/gl.h"
#include "res/res.h"

#include "main/types_ops.h"
#include "main/types_io.h"

#include <list>
#include <cmath>
#include <stdexcept>
#include <iterator>

// Helper functions to replace GLU quadric functions
namespace {

// Simple sphere rendering for mesh generation
void render_sphere(double radius, int slices, int stacks, bool with_texture = false)
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
			
			// Texture coordinates for current vertex
			double s = (double)j / slices;
			double t = (double)i / stacks;
			double s1 = (double)(j + 1) / slices;
			double t1 = (double)(i + 1) / stacks;
			
			// First triangle
			if (with_texture) glTexCoord2d(s, t);
			glNormal3d(x * zr0, y * zr0, z0);
			glVertex3d(radius * x * zr0, radius * y * zr0, radius * z0);
			
			if (with_texture) glTexCoord2d(s, t1);
			glNormal3d(x * zr1, y * zr1, z1);
			glVertex3d(radius * x * zr1, radius * y * zr1, radius * z1);
			
			lng = 2 * PI * (double)(j + 1) / slices;
			x = cos(lng);
			y = sin(lng);
			
			if (with_texture) glTexCoord2d(s1, t1);
			glNormal3d(x * zr1, y * zr1, z1);
			glVertex3d(radius * x * zr1, radius * y * zr1, radius * z1);
			
			// Second triangle
			lng = 2 * PI * (double)j / slices;
			x = cos(lng);
			y = sin(lng);
			
			if (with_texture) glTexCoord2d(s, t);
			glNormal3d(x * zr0, y * zr0, z0);
			glVertex3d(radius * x * zr0, radius * y * zr0, radius * z0);
			
			lng = 2 * PI * (double)(j + 1) / slices;
			x = cos(lng);
			y = sin(lng);
			
			if (with_texture) glTexCoord2d(s1, t1);
			glNormal3d(x * zr1, y * zr1, z1);
			glVertex3d(radius * x * zr1, radius * y * zr1, radius * z1);
			
			if (with_texture) glTexCoord2d(s1, t);
			glNormal3d(x * zr0, y * zr0, z0);
			glVertex3d(radius * x * zr0, radius * y * zr0, radius * z0);
		}
	}
	
	glEnd();
}

// Simple cylinder rendering for mesh generation
void render_cylinder(double base, double top, double height, int slices, int stacks)
{
	const double PI = 3.14159265358979323846;
	
	glBegin(GL_TRIANGLES);
	
	for (int i = 0; i < stacks; ++i) {
		double z0 = height * (double)i / stacks;
		double z1 = height * (double)(i + 1) / stacks;
		double r0 = base + (top - base) * (double)i / stacks;
		double r1 = base + (top - base) * (double)(i + 1) / stacks;
		
		for (int j = 0; j < slices; ++j) {
			double lng0 = 2 * PI * (double)j / slices;
			double lng1 = 2 * PI * (double)(j + 1) / slices;
			
			double x0 = cos(lng0);
			double y0 = sin(lng0);
			double x1 = cos(lng1);
			double y1 = sin(lng1);
			
			// Side surface triangles
			// First triangle
			glNormal3d(x0, y0, 0);
			glVertex3d(r0 * x0, r0 * y0, z0);
			
			glNormal3d(x0, y0, 0);
			glVertex3d(r1 * x0, r1 * y0, z1);
			
			glNormal3d(x1, y1, 0);
			glVertex3d(r1 * x1, r1 * y1, z1);
			
			// Second triangle
			glNormal3d(x0, y0, 0);
			glVertex3d(r0 * x0, r0 * y0, z0);
			
			glNormal3d(x1, y1, 0);
			glVertex3d(r1 * x1, r1 * y1, z1);
			
			glNormal3d(x1, y1, 0);
			glVertex3d(r0 * x1, r0 * y1, z0);
		}
	}
	
	glEnd();
}

} // anonymous namespace

namespace reaper {
namespace {
	debug::DebugOutput dout("gfx::Object",0);
}
namespace gfx {
namespace mesh {
using namespace std;

const string Sheep::name("sheep");

Sheep::Sheep()
{
	Material material;

	material.ambient_color = Color(.2,.2,.2);
	material.diffuse_color = Color(1,1,1);
	material.emission_color = Color(0,0,0);
	material.specular_color = Color(.6,.6,.6);
	material.shininess = 80;

	list.begin();
		material.use();								
		glRotatef(90,1,0,0); // rotate so z axis points downwards
		render_sphere(1, 20, 20, true); // Main body with texture
		
		material.diffuse_color = Color(0,0,0);
		material.use();

		glTranslatef(0,-.9,.2);
		render_sphere(.2, 10, 10, false); // Head without texture
		glTranslatef(0,.9,-.2);

		glTranslatef(.5,.5,0);			
		render_cylinder(.1,.1,1,8,1); // Legs
		glTranslatef(-1,0,0);
		render_cylinder(.1,.1,1,8,1);
		glTranslatef(0,-1,0);
		render_cylinder(.1,.1,1,8,1);
		glTranslatef(1,0,0);
		render_cylinder(.1,.1,1,8,1);
	list.end();		

	throw_on_gl_error("Sheep::Sheep()");
}

void Sheep::render(int lod) const
{
	list.call();
	throw_on_gl_error("Sheep::render()");
}

}
}
}

/*
 * $Author: fizzgig $
 * $Date: 2002/09/23 12:08:43 $
 * $Log: mesh.cpp,v $
 * Revision 1.21  2002/09/23 12:08:43  fizzgig
 * Moved all anonymous namespaces into the reaper-namespace
 *
 * Revision 1.20  2002/04/23 22:50:18  fizzgig
 * mesh-loading cleanup
 *
 * Revision 1.19  2002/03/04 10:22:49  fizzgig
 * sheep!
 *
 * Revision 1.18  2002/02/08 11:27:52  macke
 * rendering reorg and silhouette shadows!
 *
 * Revision 1.17  2002/01/17 04:56:56  peter
 * gcc-2 is happier this way..
 *
 * Revision 1.16  2002/01/16 00:25:27  picon
 * terr�ngbelysning
 *
 * Revision 1.15  2001/07/30 23:43:17  macke
 * H�pp, d� var det k�rt.
 *
 * Revision 1.14  2001/07/23 23:48:10  macke
 * Slimmad grafikhantering samt lite namnbyten
 *
 * Revision 1.39  2001/07/09 13:33:06  peter
 * gcc-3.0 fixar
 *
 * Revision 1.38  2001/06/07 05:14:19  macke
 * Reaper v0.9
 *
 * Revision 1.37  2001/05/15 01:42:26  peter
 * minskade ner p� debugutskrifterna
 *
 * Revision 1.36  2001/05/15 00:06:24  peter
 * bitr�taburk, jag flyttar!
 *
 * Revision 1.35  2001/05/10 11:40:14  macke
 * h�pp
 *
 */

