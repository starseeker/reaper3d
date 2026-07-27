/*
 * GLFW-based graphics driver implementation
 * Direct GLFW integration - plugin architecture removed
 */

#include "hw/compat.h"

#include <string>
#include <iostream>
#include <cstring>

#include "hw/gfx.h"
#include "hw/gfx_driver.h"
#include "hw/gfx_types.h"
#include "hw/debug.h"
#include "hw/interfaces.h"
#include "hw/exceptions.h"
#include "hw/gl.h"
#include "hw/glfw.h"

#include <GLFW/glfw3.h>

namespace reaper
{
namespace hw
{
namespace event { void setup_glfw_callbacks(GLFWwindow* window); }
namespace lowlevel
{

using std::string;

namespace { debug::DebugOutput derr("hw::gfx::glfw"); }

// GLFW error callback
void glfw_error_callback(int error, const char* description)
{
    derr << "GLFW Error " << error << ": " << description << "\n";
}

class Gfx_glfw : public Gfx_driver
{
    GLFWwindow* window;
    Gfx_driver_data data;
    ifs::Gfx* main;
    
public:
    Gfx_glfw(ifs::Gfx* m);
    ~Gfx_glfw();
    
    Gfx_driver_data* internal_data() { return &data; }
    void update_screen();
    bool setup_mode(const gfx::VideoMode& mode);
    void restore_mode();
    
private:
    bool initialize_glfw();
    void shutdown_glfw();
    void destroy_window();
    GLFWwindow* create_window(const gfx::VideoMode& mode);
    void setup_opengl_context();
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void window_focus_callback(GLFWwindow* window, int focused);
};

Gfx_glfw::Gfx_glfw(ifs::Gfx* m)
    : window(nullptr), data(), main(m)
{
    derr << "Initializing GLFW graphics driver\n";
    
    if (!initialize_glfw()) {
        throw hw_fatal_error("Failed to initialize GLFW");
    }
    
    data.desktop = gfx::VideoMode(800, 600, 24, false);

    // Get desktop resolution
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (primary) {
        const GLFWvidmode* desktop_mode = glfwGetVideoMode(primary);
        if (desktop_mode) {
            data.desktop.width = desktop_mode->width;
            data.desktop.height = desktop_mode->height;
            data.desktop.depth = desktop_mode->redBits + desktop_mode->greenBits + desktop_mode->blueBits;
            data.desktop.fullscreen = false; // Start in windowed mode
            
            derr << "Desktop resolution: " << data.desktop.width << "x" << data.desktop.height 
                 << " (" << data.desktop.depth << " bits)\n";
        }
    }

    // Add common modes even if monitor enumeration was unavailable.
    main->add_mode(gfx::VideoMode(800, 600, data.desktop.depth, false));
    main->add_mode(gfx::VideoMode(1024, 768, data.desktop.depth, false));
    main->add_mode(gfx::VideoMode(1280, 720, data.desktop.depth, false));
    main->add_mode(gfx::VideoMode(1920, 1080, data.desktop.depth, false));
    main->add_mode(gfx::VideoMode(data.desktop.width, data.desktop.height, data.desktop.depth, true));
    
    derr << "GLFW graphics driver initialized\n";
}

Gfx_glfw::~Gfx_glfw()
{
    derr << "Shutting down GLFW graphics driver\n";
    shutdown_glfw();
}

bool Gfx_glfw::initialize_glfw()
{
    // Set error callback before initialization
    glfwSetErrorCallback(glfw_error_callback);
    
    if (!glfwInit()) {
        derr << "Failed to initialize GLFW\n";
        return false;
    }
    
    // Set OpenGL context hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    
    return true;
}

void Gfx_glfw::shutdown_glfw()
{
    destroy_window();
    glfwTerminate();
}

void Gfx_glfw::destroy_window()
{
    if (!window) {
        return;
    }

    glfw::set_current_window(nullptr);
    glfwMakeContextCurrent(nullptr);
    glfwDestroyWindow(window);
    window = nullptr;
    data.window_active = false;
}

GLFWwindow* Gfx_glfw::create_window(const gfx::VideoMode& mode)
{
    GLFWmonitor* monitor = mode.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    
    derr << "Creating window: " << mode.width << "x" << mode.height 
         << (mode.fullscreen ? " (fullscreen)" : " (windowed)") << "\n";
    
    GLFWwindow* win = glfwCreateWindow(mode.width, mode.height, "Reaper3D", monitor, nullptr);
    
    if (!win) {
        derr << "Failed to create GLFW window\n";
        return nullptr;
    }
    
    // Make the OpenGL context current
    glfwMakeContextCurrent(win);
    
    // Enable v-sync
    glfwSwapInterval(1);
    
    return win;
}

void Gfx_glfw::setup_opengl_context()
{
    if (!window) return;
    
    glfwMakeContextCurrent(window);

    // OpenGL state belongs to the renderer.  In particular, enabling depth
    // testing here makes the menu background occlude all later UI at z=0.

    // Check for alpha and stencil buffer support
    GLint alpha_bits = 0, stencil_bits = 0;
    glGetIntegerv(GL_ALPHA_BITS, &alpha_bits);
    glGetIntegerv(GL_STENCIL_BITS, &stencil_bits);
    
    data.alpha = (alpha_bits > 0);
    data.stencil = (stencil_bits > 0);

    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    data.is_accelerated =
        renderer != nullptr &&
        std::strstr(renderer, "llvmpipe") == nullptr &&
        std::strstr(renderer, "softpipe") == nullptr &&
        std::strstr(renderer, "Software Rasterizer") == nullptr;

    int framebuffer_width = 0;
    int framebuffer_height = 0;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    data.current.width = framebuffer_width;
    data.current.height = framebuffer_height;
    glViewport(0, 0, framebuffer_width, framebuffer_height);

    derr << "OpenGL context: renderer=" << (renderer ? renderer : "unknown")
         << ", accelerated=" << data.is_accelerated
         << ", alpha=" << data.alpha << ", stencil=" << data.stencil
         << ", framebuffer=" << framebuffer_width << "x" << framebuffer_height << "\n";
}

bool Gfx_glfw::setup_mode(const gfx::VideoMode& mode)
{
    derr << "Setting up video mode: " << mode.width << "x" << mode.height << "\n";
    
    destroy_window();
    
    // Create new window with requested mode
    window = create_window(mode);
    if (!window) {
        return false;
    }
    
    // Update the global current window reference
    glfw::set_current_window(window);
    
    // Setup GLFW event callbacks for input handling
    glfw::setup_glfw_callbacks(window);

    // Store current mode
    data.current = mode;
    if (data.current.depth == 0) {
        data.current.depth = data.desktop.depth;
    }
    data.window_active = true;

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowFocusCallback(window, window_focus_callback);
    
    // Set up OpenGL context
    setup_opengl_context();
    
    derr << "Video mode set successfully\n";
    return true;
}

void Gfx_glfw::restore_mode()
{
    if (window) {
        derr << "Restoring video mode\n";
        destroy_window();
    }
}

void Gfx_glfw::update_screen()
{
    if (window) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Gfx_glfw::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    Gfx_glfw* self = static_cast<Gfx_glfw*>(glfwGetWindowUserPointer(window));
    if (!self || width <= 0 || height <= 0) {
        return;
    }

    self->data.current.width = width;
    self->data.current.height = height;
    glViewport(0, 0, width, height);
}

void Gfx_glfw::window_focus_callback(GLFWwindow* window, int focused)
{
    Gfx_glfw* self = static_cast<Gfx_glfw*>(glfwGetWindowUserPointer(window));
    if (self) {
        self->data.window_active = focused == GLFW_TRUE;
    }
}

} // namespace lowlevel

// GLFW utility functions implementation
namespace glfw {

static GLFWwindow* current_window = nullptr;

bool initialize_glfw()
{
    glfwSetErrorCallback(lowlevel::glfw_error_callback);
    if (!glfwInit()) {
        return false;
    }
    
    // Set default OpenGL context hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    
    return true;
}

void terminate_glfw()
{
    if (current_window) {
        glfwDestroyWindow(current_window);
        current_window = nullptr;
    }
    glfwTerminate();
}

GLFWwindow* get_current_window()
{
    return current_window;
}

void set_current_window(GLFWwindow* window)
{
    current_window = window;
}

void setup_glfw_callbacks(GLFWwindow* window)
{
    // Forward to the event namespace implementation
    event::setup_glfw_callbacks(window);
}

} // namespace glfw

// Factory function for creating GLFW graphics driver
// This will be called by the graphics system
extern "C" lowlevel::Gfx_driver* create_gfx_glfw(ifs::Gfx* m)
{
    try {
        auto* driver = new lowlevel::Gfx_glfw(m);
        return driver;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create GLFW graphics driver: " << e.what() << std::endl;
        return nullptr;
    }
}

} // namespace hw
} // namespace reaper
