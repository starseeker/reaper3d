/*
 * Modern GLSL shader test - demonstrates OpenGL 2.0+ shader pipeline
 * Shows both fixed pipeline and modern shader rendering side by side
 */

#include <iostream>
#include <fstream>
#include <cmath>

#ifdef HAVE_OSMESA
#include <GL/osmesa.h>
#include <GL/gl.h>
#endif

#include "../gfx/modern_shaders.h"
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

void setup_identity_matrix(float* matrix)
{
    // Set up identity matrix
    for (int i = 0; i < 16; i++) {
        matrix[i] = 0.0f;
    }
    matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
}

void draw_with_fixed_pipeline()
{
    std::cout << "Drawing with fixed pipeline (left side)...\n";
    
    using namespace reaper::gfx::vbo;
    
    // Draw on the left side of the screen
    glViewport(0, 0, 256, 512);
    
    // Draw a triangle with fixed pipeline
    SimpleVBO triangle(GL_TRIANGLES);
    triangle.add_vertex(-0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f);   // Red top
    triangle.add_vertex(-0.8f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f);  // Green bottom left  
    triangle.add_vertex(-0.2f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f);  // Blue bottom right
    triangle.upload();
    triangle.render();
    
    // Draw some lines
    SimpleVBO lines(GL_LINES);
    for (int i = 0; i < 5; i++) {
        float x = -0.8f + i * 0.3f;
        lines.add_vertex(x, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f); // Yellow
        lines.add_vertex(x, 0.3f, 1.0f, 1.0f, 0.0f, 1.0f);
    }
    lines.upload();
    lines.render();
}

void draw_with_modern_shaders()
{
    std::cout << "Drawing with modern GLSL shaders (right side)...\n";
    
    using namespace reaper::gfx::modern_shaders;
    
    // Draw on the right side of the screen
    glViewport(256, 0, 256, 512);
    
    // Create shader manager and load basic shader
    ShaderManager shader_mgr;
    
    if (ShaderManager::is_glsl_supported()) {
        GLSLShader* basic_shader = shader_mgr.create_shader("basic",
                                                          builtin::basic_vertex_shader,
                                                          builtin::basic_fragment_shader);
        
        if (basic_shader && basic_shader->is_ready()) {
            // Use the shader
            basic_shader->use();
            
            // Set up model-view-projection matrix (identity for this test)
            float mvp_matrix[16];
            setup_identity_matrix(mvp_matrix);
            basic_shader->set_uniform_matrix4("mvpMatrix", mvp_matrix);
            
            // Note: For a full implementation, we'd need to set up vertex attributes
            // For now, fall back to immediate mode to show the shader is working
            
            glBegin(GL_TRIANGLES);
                glColor3f(1.0f, 0.5f, 0.0f); // Orange
                glVertex2f(0.2f, 0.5f);      
                glVertex2f(-0.1f, -0.5f);    
                glVertex2f(0.5f, -0.5f);     
            glEnd();
            
            glBegin(GL_QUADS);
                glColor3f(0.5f, 0.0f, 1.0f); // Purple
                glVertex2f(0.6f, 0.2f);
                glVertex2f(0.8f, 0.2f);
                glVertex2f(0.8f, 0.0f);
                glVertex2f(0.6f, 0.0f);
            glEnd();
        } else {
            std::cout << "Failed to create basic shader, using fixed pipeline fallback\n";
            
            // Fallback to fixed pipeline
            glBegin(GL_TRIANGLES);
                glColor3f(0.8f, 0.2f, 0.2f); // Dark red
                glVertex2f(0.2f, 0.5f);      
                glVertex2f(-0.1f, -0.5f);    
                glVertex2f(0.5f, -0.5f);     
            glEnd();
        }
        
        // Return to fixed pipeline
        shader_mgr.use_fixed_pipeline();
    } else {
        std::cout << "GLSL not supported, using fixed pipeline fallback\n";
        
        // Fallback rendering
        glBegin(GL_TRIANGLES);
            glColor3f(0.5f, 0.5f, 0.5f); // Gray
            glVertex2f(0.2f, 0.5f);      
            glVertex2f(-0.1f, -0.5f);    
            glVertex2f(0.5f, -0.5f);     
        glEnd();
    }
}

int main()
{
#ifdef HAVE_OSMESA
    const int width = 512;
    const int height = 512;
    
    std::cout << "Modern shader test (" << width << "x" << height << ")...\n";
    
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
    glClearColor(0.1f, 0.1f, 0.3f, 1.0f); // Dark blue background
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
    
    // Test both rendering approaches
    draw_with_fixed_pipeline();    // Left side
    draw_with_modern_shaders();    // Right side
    
    // Draw a dividing line in the middle
    glViewport(0, 0, width, height);
    glColor3f(1.0f, 1.0f, 1.0f); // White
    glBegin(GL_LINES);
        glVertex2f(0.0f, -1.0f);
        glVertex2f(0.0f, 1.0f);
    glEnd();
    
    // Flush all drawing commands
    glFlush();
    
    // Save the rendered image
    if (save_ppm("shader_test.ppm", width, height, buffer)) {
        std::cout << "Shader comparison image saved to shader_test.ppm\n";
    } else {
        std::cerr << "Failed to save image\n";
    }
    
    // Clean up
    delete[] buffer;
    OSMesaDestroyContext(ctx);
    
    std::cout << "Modern shader test completed successfully\n";
    return 0;
    
#else
    std::cerr << "OSMesa support not compiled in\n";
    return 1;
#endif
}