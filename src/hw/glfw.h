/*
 * GLFW Integration Header
 * 
 * This header provides access to GLFW functionality for the Reaper3D engine.
 * It includes function prototypes and utilities for the GLFW migration.
 */

#ifndef REAPER_HW_GLFW_H
#define REAPER_HW_GLFW_H

struct GLFWwindow;

namespace reaper {
namespace hw {
namespace ifs { class Gfx; class Event; }
namespace lowlevel { class Gfx_driver; }

namespace glfw {

// Forward declaration for internal use
void set_current_window(GLFWwindow* window);

// GLFW initialization and cleanup
bool initialize_glfw();
void terminate_glfw();

// Window management
GLFWwindow* get_current_window();

// Event handling functions
void setup_glfw_callbacks(GLFWwindow* window);
void process_glfw_events();
bool should_close_window();

} // namespace glfw

} // namespace hw
} // namespace reaper

// Factory functions for GLFW drivers (defined in implementation files)
extern "C" {
reaper::hw::lowlevel::Gfx_driver* create_gfx_glfw(reaper::hw::ifs::Gfx* m);
void* create_event_glfw(reaper::hw::ifs::Event* m);
}

#endif // REAPER_HW_GLFW_H