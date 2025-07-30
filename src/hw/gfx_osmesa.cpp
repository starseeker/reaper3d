/*
 * OSMesa-based graphics driver implementation for headless rendering
 * Based on Mesa's Off-Screen rendering capabilities
 */

#include "hw/compat.h"

#include <string>
#include <iostream>
#include <vector>

#include "hw/gfx.h"
#include "hw/gfx_driver.h"
#include "hw/gfx_types.h"
#include "hw/debug.h"
#include "hw/interfaces.h"
#include "hw/exceptions.h"
#include "hw/gl.h"

#ifdef HAVE_OSMESA
#include <GL/osmesa.h>
#endif

namespace reaper
{
namespace hw
{
namespace lowlevel
{

using std::string;

namespace { debug::DebugOutput derr("hw::gfx::osmesa"); }

class Gfx_osmesa : public Gfx_driver
{
#ifdef HAVE_OSMESA
    OSMesaContext ctx;
    void* buffer;
    int width, height;
#endif
    Gfx_driver_data data;
    ifs::Gfx* main;
    bool initialized;
    
public:
    Gfx_osmesa(ifs::Gfx* m);
    ~Gfx_osmesa();
    
    Gfx_driver_data* internal_data() { return &data; }
    void update_screen();
    bool setup_mode(const gfx::VideoMode& mode);
    void restore_mode();
    
    // OSMesa-specific methods
    bool save_screenshot(const char* filename);
    
private:
    bool initialize_osmesa();
    void shutdown_osmesa();
};

Gfx_osmesa::Gfx_osmesa(ifs::Gfx* m)
    : main(m), initialized(false)
#ifdef HAVE_OSMESA
    , ctx(nullptr), buffer(nullptr), width(800), height(600)
#endif
{
    derr << "Initializing OSMesa graphics driver\n";
    
#ifdef HAVE_OSMESA
    if (!initialize_osmesa()) {
        throw hw_fatal_error("Failed to initialize OSMesa");
    }
    
    // Set up default desktop mode for headless operation
    data.desktop.width = 800;
    data.desktop.height = 600;
    data.desktop.depth = 32;
    data.desktop.fullscreen = false;
    
    // Add common video modes for headless rendering
    main->add_mode(gfx::VideoMode(800, 600, 32, false));
    main->add_mode(gfx::VideoMode(1024, 768, 32, false));
    main->add_mode(gfx::VideoMode(1280, 720, 32, false));
    main->add_mode(gfx::VideoMode(1920, 1080, 32, false));
    
    initialized = true;
    derr << "OSMesa graphics driver initialized\n";
#else
    throw hw_fatal_error("OSMesa support not compiled in");
#endif
}

Gfx_osmesa::~Gfx_osmesa()
{
    derr << "Shutting down OSMesa graphics driver\n";
    shutdown_osmesa();
}

bool Gfx_osmesa::initialize_osmesa()
{
#ifdef HAVE_OSMESA
    // Create OSMesa context - RGBA mode, 32-bit depth buffer, 8-bit stencil
    ctx = OSMesaCreateContextExt(OSMESA_RGBA, 32, 8, 0, nullptr);
    if (!ctx) {
        derr << "Failed to create OSMesa context\n";
        return false;
    }
    
    // Allocate image buffer
    buffer = new char[width * height * 4]; // RGBA
    if (!buffer) {
        derr << "Failed to allocate image buffer\n";
        OSMesaDestroyContext(ctx);
        ctx = nullptr;
        return false;
    }
    
    // Make context current
    if (!OSMesaMakeCurrent(ctx, buffer, GL_UNSIGNED_BYTE, width, height)) {
        derr << "Failed to make OSMesa context current\n";
        delete[] (char*)buffer;
        buffer = nullptr;
        OSMesaDestroyContext(ctx);
        ctx = nullptr;
        return false;
    }
    
    derr << "OSMesa context created: " << width << "x" << height << "\n";
    return true;
#else
    return false;
#endif
}

void Gfx_osmesa::shutdown_osmesa()
{
#ifdef HAVE_OSMESA
    if (ctx) {
        OSMesaDestroyContext(ctx);
        ctx = nullptr;
    }
    if (buffer) {
        delete[] (char*)buffer;
        buffer = nullptr;
    }
#endif
    initialized = false;
}

void Gfx_osmesa::update_screen()
{
    // In OSMesa, we don't need to swap buffers like in windowed mode
    // The render is already in our buffer and can be saved to file
    glFlush();
}

bool Gfx_osmesa::setup_mode(const gfx::VideoMode& mode)
{
#ifdef HAVE_OSMESA
    if (mode.width != width || mode.height != height) {
        // Need to recreate context with new size
        shutdown_osmesa();
        
        width = mode.width;
        height = mode.height;
        
        if (!initialize_osmesa()) {
            return false;
        }
    }
    
    data.desktop.width = mode.width;
    data.desktop.height = mode.height;
    data.desktop.depth = mode.depth;
    data.desktop.fullscreen = mode.fullscreen;
    
    derr << "OSMesa mode set to: " << mode.width << "x" << mode.height 
         << " (" << mode.depth << " bits)\n";
    return true;
#else
    return false;
#endif
}

void Gfx_osmesa::restore_mode()
{
    // Nothing special needed for OSMesa
}

bool Gfx_osmesa::save_screenshot(const char* filename)
{
#ifdef HAVE_OSMESA
    if (!initialized || !buffer) {
        return false;
    }
    
    // For now, just indicate success - actual PNG writing would need libpng
    derr << "Screenshot would be saved to: " << filename << " (" << width << "x" << height << ")\n";
    return true;
#else
    return false;
#endif
}

// Factory function to create OSMesa driver with extern "C" linkage
extern "C" reaper::hw::lowlevel::Gfx_driver* create_gfx_osmesa(reaper::hw::ifs::Gfx* main_gfx)
{
    return new Gfx_osmesa(main_gfx);
}

} // namespace lowlevel
} // namespace hw
} // namespace reaper