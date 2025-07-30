/*
 * OpenGL 2.0+ VBO utilities for modernizing immediate mode rendering
 * Provides simple wrapper classes for vertex buffer objects
 */

#ifndef REAPER_GFX_VBO_UTILS_H
#define REAPER_GFX_VBO_UTILS_H

#include "hw/gl.h"
#include <vector>

namespace reaper {
namespace gfx {
namespace vbo {

// Simple vertex structure for position + color
struct PosColorVertex {
    float x, y, z;
    float r, g, b, a;
    
    PosColorVertex(float x_, float y_, float z_, float r_, float g_, float b_, float a_)
        : x(x_), y(y_), z(z_), r(r_), g(g_), b(b_), a(a_) {}
    
    PosColorVertex(float x_, float y_, float r_, float g_, float b_, float a_)
        : x(x_), y(y_), z(0.0f), r(r_), g(g_), b(b_), a(a_) {}
};

// Simple VBO wrapper for lines, triangles, etc.
class SimpleVBO {
private:
    GLuint vbo;
    GLuint vao; // Vertex Array Object for OpenGL 3.0+ compatibility
    std::vector<PosColorVertex> vertices;
    GLenum draw_mode;
    bool uploaded;
    
public:
    SimpleVBO(GLenum mode = GL_LINES);
    ~SimpleVBO();
    
    // Add vertices
    void add_vertex(float x, float y, float z, float r, float g, float b, float a);
    void add_vertex(float x, float y, float r, float g, float b, float a);
    
    // Clear vertices
    void clear();
    
    // Upload to GPU
    void upload();
    
    // Render
    void render();
    
private:
    void cleanup();
};

// Utility functions for converting immediate mode to VBO
namespace immediate {
    
// Helper class to capture immediate mode calls and convert to VBO
class ImmediateToVBO {
private:
    std::vector<PosColorVertex> vertices;
    float current_color[4];
    GLenum current_mode;
    bool in_begin;
    
public:
    ImmediateToVBO();
    
    void begin(GLenum mode);
    void end();
    void color3f(float r, float g, float b);
    void color4f(float r, float g, float b, float a);
    void vertex2f(float x, float y);
    void vertex3f(float x, float y, float z);
    
    // Get the captured vertices
    const std::vector<PosColorVertex>& get_vertices() const { return vertices; }
    GLenum get_mode() const { return current_mode; }
    
    void clear();
};

} // namespace immediate
} // namespace vbo
} // namespace gfx
} // namespace reaper

#endif // REAPER_GFX_VBO_UTILS_H