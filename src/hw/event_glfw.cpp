/*
 * GLFW-based event handling implementation
 * Direct GLFW integration - plugin architecture removed
 */


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

namespace {

std::queue<Event> keyboard_events;
std::queue<Event> mouse_events;
bool window_should_close = false;

void clear_events(std::queue<Event>& events)
{
    std::queue<Event> empty;
    events.swap(empty);
}

id::EventID translate_key(int key)
{
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
        // GLFW uses ASCII values for printable keys.
        if (key >= 32 && key <= 126) {
            return static_cast<id::EventID>(key);
        }
        return id::Unknown;
    }
}

} // anonymous namespace

class KeyboardGLFW : public InputDevice {
protected:
    bool poll_device();

public:
    KeyboardGLFW(const Gfx_driver_data*, ifs::Event*) {
        derr << "GLFW keyboard initialized\n";
    }

    virtual ~KeyboardGLFW() {
        derr << "GLFW keyboard shutdown\n";
    }
};

bool KeyboardGLFW::poll_device()
{
    if (keyboard_events.empty()) {
        return false;
    }
    pending.push(keyboard_events.front());
    keyboard_events.pop();
    return true;
}

class MouseGLFW : public InputDevice {
protected:
    bool poll_device();

public:
    MouseGLFW(const Gfx_driver_data*, ifs::Event*) {
        derr << "GLFW mouse initialized\n";
    }

    virtual ~MouseGLFW() {
        derr << "GLFW mouse shutdown\n";
    }
};

bool MouseGLFW::poll_device()
{
    if (mouse_events.empty()) {
        return false;
    }
    pending.push(mouse_events.front());
    mouse_events.pop();
    return true;
}

void key_callback(GLFWwindow*, int key, int, int action, int)
{
    if (action == GLFW_REPEAT) {
        return;
    }

    id::EventID translated = translate_key(key);
    if (translated != id::Unknown) {
        keyboard_events.push(Event(translated, action == GLFW_PRESS));
    }
}

void mouse_button_callback(GLFWwindow*, int button, int action, int)
{
    if (action != GLFW_REPEAT) {
        mouse_events.push(Event(button, action == GLFW_PRESS));
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    int width = 0;
    int height = 0;
    glfwGetWindowSize(window, &width, &height);
    if (width <= 0 || height <= 0) {
        return;
    }

    const float x = static_cast<float>((2.0 * xpos) / width - 1.0);
    const float y = static_cast<float>((2.0 * ypos) / height - 1.0);
    mouse_events.push(Event(0, x));
    mouse_events.push(Event(1, y));
}

void window_close_callback(GLFWwindow*)
{
    derr << "Window close requested\n";
    window_should_close = true;
    keyboard_events.push(Event(id::Escape, true));
    keyboard_events.push(Event(id::Escape, false));
}

// Input device module for GLFW.
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

    window_should_close = false;
    clear_events(keyboard_events);
    clear_events(mouse_events);

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
reaper::hw::event::InputDeviceModule* create_event_glfw(reaper::hw::ifs::Event* m)
{
    return new reaper::hw::event::IDM_GLFW(m);
}
}
