#include "hw/gl.h"

#include <cstdio>
#include <cstdlib>

#include <GLFW/glfw3.h>

namespace {

template<typename Proc>
Proc load_gl_function(const char* core_name, const char* extension_name)
{
	GLFWglproc address = glfwGetProcAddress(core_name);
	if (address == nullptr)
		address = glfwGetProcAddress(extension_name);
	if (address == nullptr) {
		std::fprintf(
			stderr,
			"Required OpenGL function is unavailable: %s / %s\n",
			core_name,
			extension_name);
		std::abort();
	}
	return reinterpret_cast<Proc>(address);
}

}

// Keep the historical ARB names used by the renderer while dispatching
// through the current context's core or extension entry point.
extern "C" {

void glClientActiveTextureARB(GLenum texture)
{
	using Proc = void (APIENTRY*)(GLenum);
	static const Proc proc =
		load_gl_function<Proc>("glClientActiveTexture", "glClientActiveTextureARB");
	proc(texture);
}

void glMultiTexCoord2fARB(GLenum target, GLfloat s, GLfloat t)
{
	using Proc = void (APIENTRY*)(GLenum, GLfloat, GLfloat);
	static const Proc proc =
		load_gl_function<Proc>("glMultiTexCoord2f", "glMultiTexCoord2fARB");
	proc(target, s, t);
}

void glActiveTextureARB(GLenum texture)
{
	using Proc = void (APIENTRY*)(GLenum);
	static const Proc proc =
		load_gl_function<Proc>("glActiveTexture", "glActiveTextureARB");
	proc(texture);
}

}
