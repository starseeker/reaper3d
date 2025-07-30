#include "hw/compat.h"

#include "gfx/exceptions.h"
#include "gfx/settings.h"

#include "hw/gl.h"

namespace reaper {
namespace gfx {   

const bool *gl_error::test_for_errors = &Settings::current.test_for_errors;

// Simple replacement for gluErrorString
const char* gl_error_string(GLenum error)
{
	switch(error) {
	case GL_NO_ERROR:
		return "GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_STACK_OVERFLOW:
		return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:
		return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	default:
		return "Unknown OpenGL error";
	}
}

void throw_on_gl_error_check(const std::string &s)
{                
        GLenum err = glGetError();
	if(err) {
		const char *gl_err_str = gl_error_string(err);
                throw gfx_fatal_error(s + " - OpenGL error: " + gl_err_str);
        }
}

}
}
