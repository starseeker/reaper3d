/*
 * OpenGL 2.0+ VBO utilities implementation
 */

#include "hw/compat.h"
#include "gfx/vbo_utils.h"
#include "hw/debug.h"
#include "hw/gl.h"

namespace reaper {
namespace gfx {
namespace vbo {

namespace { debug::DebugOutput dout("gfx::vbo"); }

SimpleVBO::SimpleVBO(GLenum mode)
    : vbo(0), vao(0), draw_mode(mode), uploaded(false)
{
    // VBO support will be added later with proper extension detection
    // For now, this acts as a vertex container for immediate mode rendering
}

SimpleVBO::~SimpleVBO()
{
    cleanup();
}

void SimpleVBO::cleanup()
{
    // VBO cleanup will be implemented when VBO support is added
}

void SimpleVBO::upload()
{
    // Upload will be implemented when VBO support is added
    // For now, data is kept in vertices vector for immediate mode
    uploaded = true;
}

void SimpleVBO::add_vertex(float x, float y, float z, float r, float g, float b, float a)
{
    vertices.emplace_back(x, y, z, r, g, b, a);
    uploaded = false;
}

void SimpleVBO::add_vertex(float x, float y, float r, float g, float b, float a)
{
    vertices.emplace_back(x, y, 0.0f, r, g, b, a);
    uploaded = false;
}

void SimpleVBO::clear()
{
    vertices.clear();
    uploaded = false;
}

void SimpleVBO::render()
{
    if (vertices.empty()) {
        return;
    }
    
    // For now, use immediate mode as fallback since VBO extensions aren't set up
    // TODO: Add proper VBO extension detection and initialization
    glBegin(draw_mode);
    for (const auto& vertex : vertices) {
        glColor4f(vertex.r, vertex.g, vertex.b, vertex.a);
        glVertex3f(vertex.x, vertex.y, vertex.z);
    }
    glEnd();
}

namespace immediate {

ImmediateToVBO::ImmediateToVBO()
    : current_mode(GL_TRIANGLES), in_begin(false)
{
    current_color[0] = 1.0f;
    current_color[1] = 1.0f;
    current_color[2] = 1.0f;
    current_color[3] = 1.0f;
}

void ImmediateToVBO::begin(GLenum mode)
{
    if (in_begin) {
        dout << "Warning: begin() called while already in begin()\n";
        return;
    }
    current_mode = mode;
    in_begin = true;
    vertices.clear();
}

void ImmediateToVBO::end()
{
    if (!in_begin) {
        dout << "Warning: end() called without begin()\n";
        return;
    }
    in_begin = false;
}

void ImmediateToVBO::color3f(float r, float g, float b)
{
    current_color[0] = r;
    current_color[1] = g;
    current_color[2] = b;
    current_color[3] = 1.0f;
}

void ImmediateToVBO::color4f(float r, float g, float b, float a)
{
    current_color[0] = r;
    current_color[1] = g;
    current_color[2] = b;
    current_color[3] = a;
}

void ImmediateToVBO::vertex2f(float x, float y)
{
    if (!in_begin) {
        dout << "Warning: vertex2f() called outside begin/end\n";
        return;
    }
    vertices.emplace_back(x, y, 0.0f, 
                         current_color[0], current_color[1], 
                         current_color[2], current_color[3]);
}

void ImmediateToVBO::vertex3f(float x, float y, float z)
{
    if (!in_begin) {
        dout << "Warning: vertex3f() called outside begin/end\n";
        return;
    }
    vertices.emplace_back(x, y, z, 
                         current_color[0], current_color[1], 
                         current_color[2], current_color[3]);
}

void ImmediateToVBO::clear()
{
    vertices.clear();
    in_begin = false;
}

} // namespace immediate
} // namespace vbo
} // namespace gfx
} // namespace reaper