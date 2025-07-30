/*
 * Simple OSMesa test program to validate headless rendering
 * Based on Mesa's OSMesa demo patterns
 */

#include <iostream>
#include <fstream>
#include <cstring>

#ifdef HAVE_OSMESA
#include <GL/osmesa.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

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

int main()
{
#ifdef HAVE_OSMESA
    const int width = 512;
    const int height = 512;
    
    std::cout << "Testing OSMesa headless rendering (" << width << "x" << height << ")...\n";
    
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
    glClearColor(0.2f, 0.2f, 0.8f, 1.0f); // Blue background
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set up simple orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Draw a simple triangle using immediate mode
    glColor3f(1.0f, 0.0f, 0.0f); // Red
    glBegin(GL_TRIANGLES);
        glVertex2f(0.0f, 0.5f);   // Top
        glVertex2f(-0.5f, -0.5f); // Bottom left
        glVertex2f(0.5f, -0.5f);  // Bottom right
    glEnd();
    
    // Draw a green square
    glColor3f(0.0f, 1.0f, 0.0f); // Green
    glBegin(GL_QUADS);
        glVertex2f(-0.8f, 0.8f);  // Top left
        glVertex2f(-0.3f, 0.8f);  // Top right
        glVertex2f(-0.3f, 0.3f);  // Bottom right
        glVertex2f(-0.8f, 0.3f);  // Bottom left
    glEnd();
    
    // Flush all drawing commands
    glFlush();
    
    // Save the rendered image
    if (save_ppm("osmesa_test.ppm", width, height, buffer)) {
        std::cout << "Rendered image saved to osmesa_test.ppm\n";
    } else {
        std::cerr << "Failed to save image\n";
    }
    
    // Clean up
    delete[] buffer;
    OSMesaDestroyContext(ctx);
    
    std::cout << "OSMesa test completed successfully\n";
    return 0;
    
#else
    std::cerr << "OSMesa support not compiled in\n";
    return 1;
#endif
}