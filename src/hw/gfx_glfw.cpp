/*
 * GLFW-based graphics driver implementation
 * Migrating from X11 to GLFW for cross-platform windowing support
 */

#include "hw/compat.h"

#include <string>
#include <iostream>

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
    bool fullscreen;
    int window_width, window_height;
    
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
    GLFWwindow* create_window(const gfx::VideoMode& mode);
    void setup_opengl_context();
};

Gfx_glfw::Gfx_glfw(ifs::Gfx* m)
    : window(nullptr), main(m), fullscreen(false), window_width(800), window_height(600)
{
    derr << "Initializing GLFW graphics driver\n";
    
    if (!initialize_glfw()) {
        throw hw_fatal_error("Failed to initialize GLFW");
    }
    
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
                 
            // Add some common video modes
            main->add_mode(gfx::VideoMode(800, 600, data.desktop.depth, false));
            main->add_mode(gfx::VideoMode(1024, 768, data.desktop.depth, false));
            main->add_mode(gfx::VideoMode(1280, 720, data.desktop.depth, false));
            main->add_mode(gfx::VideoMode(1920, 1080, data.desktop.depth, false));
            main->add_mode(gfx::VideoMode(data.desktop.width, data.desktop.height, data.desktop.depth, true));
        }
    }
    
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
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    
    return true;
}

void Gfx_glfw::shutdown_glfw()
{
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
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
    
    // Set up basic OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);
    
    // Check for alpha and stencil buffer support
    GLint alpha_bits = 0, stencil_bits = 0;
    glGetIntegerv(GL_ALPHA_BITS, &alpha_bits);
    glGetIntegerv(GL_STENCIL_BITS, &stencil_bits);
    
    data.alpha = (alpha_bits > 0);
    data.stencil = (stencil_bits > 0);
    
    derr << "OpenGL context: alpha=" << data.alpha << ", stencil=" << data.stencil << "\n";
}

bool Gfx_glfw::setup_mode(const gfx::VideoMode& mode)
{
    derr << "Setting up video mode: " << mode.width << "x" << mode.height << "\n";
    
    // If we already have a window, destroy it first
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    
    // Create new window with requested mode
    window = create_window(mode);
    if (!window) {
        return false;
    }
    
    // Update the global current window reference
    glfw::set_current_window(window);
    
    // Store current mode
    window_width = mode.width;
    window_height = mode.height;
    fullscreen = mode.fullscreen;
    
    // Set up OpenGL context
    setup_opengl_context();
    
    derr << "Video mode set successfully\n";
    return true;
}

void Gfx_glfw::restore_mode()
{
    derr << "Restoring video mode\n";
    // GLFW handles mode restoration automatically when window is destroyed
}

void Gfx_glfw::update_screen()
{
    if (window) {
        glfwSwapBuffers(window);
        glfwPollEvents();
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