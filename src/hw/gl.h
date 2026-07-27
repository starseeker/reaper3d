
/*
 * Author: Peter Strand <d98peter@dtek.chalmers.se>
 *
 * Include this to use OpenGL, do not include GL/gl.h directly.
 */

#ifndef REAPER_HW_GL_H
#define REAPER_HW_GL_H

#ifdef WIN32

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#include "hw/windows.h"

#endif

#ifdef SOLARIS
#define GL_SUN_triangle_list 1
#endif

#include <GL/gl.h>
#include "ext/glh/glh_extensions.h"
#include "hw/gl_state.h"


#ifdef WIN32
inline
GLvoid* glAllocateMemoryNV(GLsizei size, GLfloat readfreq, GLfloat writefreq, GLfloat priority)
{
	return wglAllocateMemoryNV(size, readfreq, writefreq, priority);
}
inline
void glFreeMemoryNV(GLvoid *pointer)
{
	wglFreeMemoryNV(pointer);
}
#else
inline
void* glAllocateMemoryNV(int s,float r,float w,float p)
{
	(void)s; (void)r; (void)w; (void)p;
	return nullptr;
}
inline
void glFreeMemoryNV(void *pointer)
{
	(void)pointer;
}
#endif


#ifdef SOLARIS
#undef GL_ARB_multitexture

//void glClientActiveTextureARB(GLenum);
//void glActiveTextureARB(GLenum);
//void glMultiTexCoord2fARB(unsigned,int,int);
#endif

#include "hw/gl_helper.h"


namespace reaper {
namespace hw {
namespace gfx {

bool init_opengl_1_2();
void enable_extensions();


}
}
}

#endif
