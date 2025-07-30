/*
 * VBO modernization test - demonstrates converting immediate mode to VBOs
 * This shows how immediate mode OpenGL can be modernized to use VBOs
 */

#include <iostream>
#include <fstream>
#include <cmath>

#ifdef HAVE_OSMESA
#include <GL/osmesa.h>
#include <GL/gl.h>
#endif

#include "../gfx/vbo_utils.h"

bool save_ppm(const char* filename, int width, int height, unsigned char* buffer)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Write PPM header
    file << "P6\n" << width << " " << height << "\n255\n";
    
    // Write RGB data (flip Y coordinate and convert RGBA to RGB)
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 4; // RGBA
            file.write(reinterpret_cast<const char*>(&buffer[idx]), 3); // Write RGB only
        }
    }
    
    return true;
}

void draw_immediate_mode()
{
    std::cout << "Drawing with immediate mode OpenGL...\n";
    
    // Draw a triangle using old immediate mode
    glColor3f(1.0f, 0.0f, 0.0f); // Red
    glBegin(GL_TRIANGLES);
        glVertex2f(0.0f, 0.5f);   // Top
        glVertex2f(-0.5f, -0.5f); // Bottom left
        glVertex2f(0.5f, -0.5f);  // Bottom right
    glEnd();
    
    // Draw some lines
    glColor3f(0.0f, 1.0f, 0.0f); // Green
    glBegin(GL_LINES);
        for (int i = 0; i < 10; i++) {
            float x = -0.8f + i * 0.16f;
            glVertex2f(x, -0.8f);
            glVertex2f(x, -0.6f);
        }
    glEnd();
}

void draw_vbo_mode()
{
    std::cout << "Drawing with VBO (modern OpenGL)...\n";
    
    using namespace reaper::gfx::vbo;
    
    // Create VBO for triangle
    SimpleVBO triangle_vbo(GL_TRIANGLES);
    triangle_vbo.add_vertex(0.0f, 0.3f, 1.0f, 0.0f, 0.0f, 1.0f);   // Top (red)
    triangle_vbo.add_vertex(-0.3f, -0.3f, 0.0f, 1.0f, 0.0f, 1.0f); // Bottom left (green)
    triangle_vbo.add_vertex(0.3f, -0.3f, 0.0f, 0.0f, 1.0f, 1.0f);  // Bottom right (blue)
    triangle_vbo.upload();
    triangle_vbo.render();
    
    // Create VBO for lines
    SimpleVBO lines_vbo(GL_LINES);
    for (int i = 0; i < 10; i++) {
        float x = -0.8f + i * 0.16f;
        lines_vbo.add_vertex(x, 0.6f, 1.0f, 1.0f, 0.0f, 1.0f); // Yellow lines
        lines_vbo.add_vertex(x, 0.8f, 1.0f, 1.0f, 0.0f, 1.0f);
    }
    lines_vbo.upload();
    lines_vbo.render();
}

void draw_immediate_to_vbo_converter()
{
    std::cout << "Drawing with immediate-to-VBO converter...\n";
    
    using namespace reaper::gfx::vbo;
    
    // Use the converter to capture immediate mode calls
    immediate::ImmediateToVBO converter;
    
    // Simulate immediate mode calls
    converter.begin(GL_TRIANGLES);
    converter.color3f(1.0f, 0.5f, 0.0f); // Orange
    converter.vertex2f(-0.2f, 0.0f);
    converter.vertex2f(-0.4f, -0.2f);
    converter.vertex2f(0.0f, -0.2f);
    converter.end();
    
    // Convert to VBO and render
    if (!converter.get_vertices().empty()) {
        SimpleVBO converted_vbo(converter.get_mode());
        for (const auto& vertex : converter.get_vertices()) {
            converted_vbo.add_vertex(vertex.x, vertex.y, vertex.z, 
                                   vertex.r, vertex.g, vertex.b, vertex.a);
        }
        converted_vbo.upload();
        converted_vbo.render();
    }
}

int main()
{
#ifdef HAVE_OSMESA
    const int width = 512;
    const int height = 512;
    
    std::cout << "VBO modernization test (" << width << "x" << height << ")...\n";
    
    // Create OSMesa context
    OSMesaContext ctx = OSMesaCreateContextExt(OSMESA_RGBA, 16, 0, 0, nullptr);
    if (!ctx) {
        std::cerr << "Failed to create OSMesa context\n";
        return 1;
    }
    
    // Allocate image buffer
    unsigned char* buffer = new unsigned char[width * height * 4];
    memset(buffer, 0, width * height * 4);
    
    // Make context current
    if (!OSMesaMakeCurrent(ctx, buffer, GL_UNSIGNED_BYTE, width, height)) {
        std::cerr << "Failed to make OSMesa context current\n";
        delete[] buffer;
        OSMesaDestroyContext(ctx);
        return 1;
    }
    
    // Set up viewport and clear color
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f); // Dark blue background
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set up simple orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Demonstrate different rendering approaches
    draw_immediate_mode();        // Left side - old immediate mode
    draw_vbo_mode();             // Right side - modern VBO
    draw_immediate_to_vbo_converter(); // Center - converter utility
    
    // Flush all drawing commands
    glFlush();
    
    // Save the rendered image
    if (save_ppm("vbo_test.ppm", width, height, buffer)) {
        std::cout << "VBO comparison image saved to vbo_test.ppm\n";
    } else {
        std::cerr << "Failed to save image\n";
    }
    
    // Clean up
    delete[] buffer;
    OSMesaDestroyContext(ctx);
    
    std::cout << "VBO modernization test completed successfully\n";
    return 0;
    
#else
    std::cerr << "OSMesa support not compiled in\n";
    return 1;
#endif
}