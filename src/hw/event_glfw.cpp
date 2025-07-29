/*
 * GLFW-based event handling implementation
 * Migrating from X11 to GLFW for cross-platform input support
 */

#include "hw/compat.h"

#include "hw/debug.h"
#include "hw/event_prim.h"
#include "hw/glfw.h"

#include <queue>
#include <GLFW/glfw3.h>

#include "hw/gfx_types.h"
#include "hw/gfx_driver.h"
#include "hw/interfaces.h"

namespace reaper {
namespace hw {
namespace event {

namespace { debug::DebugOutput derr("hw::event::glfw"); }

using lowlevel::Gfx_driver_data;

class KeyboardGLFW : public InputDevice {
    const Gfx_driver_data* gx;
    ifs::Event* main;

protected:
    bool poll_device();
    
    id::EventID translate_key(int key) {
        switch (key) {
        case GLFW_KEY_ENTER:      return id::Enter;
        case GLFW_KEY_DOWN:       return id::Down;
        case GLFW_KEY_UP:         return id::Up;
        case GLFW_KEY_LEFT:       return id::Left;
        case GLFW_KEY_RIGHT:      return id::Right;
        case GLFW_KEY_ESCAPE:     return id::Escape;
        case GLFW_KEY_PAGE_UP:    return id::PageUp;
        case GLFW_KEY_PAGE_DOWN:  return id::PageDown;
        case GLFW_KEY_HOME:       return id::Home;
        case GLFW_KEY_END:        return id::End;
        case GLFW_KEY_INSERT:     return id::Insert;
        case GLFW_KEY_DELETE:     return id::Delete;
        case GLFW_KEY_BACKSPACE:  return id::Backspace;
        case GLFW_KEY_F1:         return id::F1;
        case GLFW_KEY_F2:         return id::F2;
        case GLFW_KEY_F3:         return id::F3;
        case GLFW_KEY_F4:         return id::F4;
        case GLFW_KEY_F5:         return id::F5;
        case GLFW_KEY_F6:         return id::F6;
        case GLFW_KEY_F7:         return id::F7;
        case GLFW_KEY_F8:         return id::F8;
        case GLFW_KEY_F9:         return id::F9;
        case GLFW_KEY_F10:        return id::F10;
        case GLFW_KEY_F11:        return id::F11;
        case GLFW_KEY_F12:        return id::F12;
        default:
            // For alphanumeric keys, GLFW uses ASCII values
            if (key >= 32 && key <= 126) {
                return static_cast<id::EventID>(key);
            }
            return id::Unknown;
        }
    }

public:
    KeyboardGLFW(const Gfx_driver_data* g, ifs::Event* m) : gx(g), main(m) {
        derr << "GLFW keyboard initialized\n";
    }

    virtual ~KeyboardGLFW() {
        derr << "GLFW keyboard shutdown\n";
    }
};

bool KeyboardGLFW::poll_device()
{
    // For now, just return false as events are handled via callbacks
    // This would be expanded later to integrate with the existing event system
    return false;
}

class MouseGLFW : public InputDevice {
    const Gfx_driver_data* gx;
    ifs::Event* main;

protected:
    bool poll_device();

public:
    MouseGLFW(const Gfx_driver_data* g, ifs::Event* m) : gx(g), main(m) {
        derr << "GLFW mouse initialized\n";
    }

    virtual ~MouseGLFW() {
        derr << "GLFW mouse shutdown\n";
    }
};

bool MouseGLFW::poll_device()
{
    // For now, just return false as events are handled via callbacks
    // This would be expanded later to integrate with the existing event system
    return false;
}

// Global event queue for GLFW callbacks (for future use)
static std::queue<Event> glfw_event_queue;
static bool window_should_close = false;

// Basic GLFW callback functions for initial testing
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        derr << "Key pressed: " << key << "\n";
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS) {
        derr << "Mouse button pressed: " << button << "\n";
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    // Mouse movement tracking (not logged to avoid spam)
}

void window_close_callback(GLFWwindow* window)
{
    derr << "Window close requested\n";
    window_should_close = true;
}

// Input device module for GLFW (similar to X11 implementation)
class IDM_GLFW : public InputDeviceModule
{
    ifs::Event* main;
public:
    IDM_GLFW(ifs::Event* m) : main(m) { }
    
    void scan_inputdevices(Gfx_driver_data* g, device_inserter out)
    {
        *out++ = new KeyboardGLFW(g, main);
        *out++ = new MouseGLFW(g, main);
        derr << "GLFW input devices initialized\n";
    }
};

// Function to setup GLFW event callbacks
void setup_glfw_callbacks(GLFWwindow* window)
{
    if (!window) return;
    
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetWindowCloseCallback(window, window_close_callback);
    
    derr << "GLFW event callbacks setup complete\n";
}

// Function to check if window should close
bool should_close_window()
{
    return window_should_close;
}

// Function to process GLFW events
void process_glfw_events()
{
    glfwPollEvents();
}

} // namespace event
} // namespace hw
} // namespace reaper

extern "C" {
void* create_event_glfw(reaper::hw::ifs::Event* m)
{
    return new reaper::hw::event::IDM_GLFW(m);
}
}