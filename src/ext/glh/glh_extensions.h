#ifndef GLH_EXTENSIONS
#define GLH_EXTENSIONS

#include <string.h>
#include <stdio.h>

#ifdef _WIN32
# include "hw/windows.h"
#else
# include <stdlib.h>
#endif

#include <GL/gl.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
# include <GL/wglext.h>
#else
# define GLH_EXT_GET_PROC_ADDRESS(p) glfwGetProcAddress(p)
#endif

#include "glh_genext.h"

int glh_init_extensions(const char *origReqExts);
const char* glh_get_unsupported_extensions();
void glh_shutdown_extensions();

#endif /* GLH_EXTENSIONS */
