# Reaper3D - Modern CMake Build

Reaper3D is a 3D space combat game that has been modernized to build with CMake and C++17 on modern Linux systems.

## Architecture

**Graphics and Input:** The engine uses GLFW directly for all graphics and input handling. The previous plugin architecture for backend selection has been removed in favor of a simplified, GLFW-only approach that provides better cross-platform compatibility and maintainability.

**Sound:** The engine uses a dummy sound system by default. While the original plugin architecture for sound drivers has been simplified, the interface remains for potential future enhancements.

## Build Requirements

### System Dependencies (Ubuntu/Debian)
```bash
sudo apt-get update && sudo apt-get install -y \
    build-essential \
    cmake \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libx11-dev \
    libxext-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libglfw3-dev \
    zlib1g-dev \
    libpng-dev \
    libopenal-dev \
    pkg-config
```

### Build Process
```bash
# Clone the repository
git clone https://github.com/starseeker/reaper3d.git
cd reaper3d

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the project
make -j$(nproc)

# The executable will be created as bin/reaper3d
```

## Modernization Progress

### ✅ Completed Modernizations

- **CMake Build System**: Replaced legacy autotools with modern CMake
- **C++17 Standard**: Updated from C++11 to C++17 
- **System Libraries**: Uses find_package() for OpenGL, X11, zlib, libpng, OpenAL, pthreads
- **Auto Pointer Updates**: Converted key auto_ptr uses to unique_ptr with make_unique
- **Template Fixes**: Fixed template-dependent name lookup for C++17 compliance
- **Deprecated Constructs**: Removed register keywords, bind1st/not1 functions, exception specifications
- **Template Specialization**: Fixed explicit specialization syntax
- **Build Infrastructure**: Added .gitignore, proper include directories

### 🚧 Partially Completed

- **Smart Pointer Migration**: Some auto_ptr instances remain in headers and complex template code
- **String Conversions**: Most ConfVal to string assignments fixed, some remain
- **Library Compilation**: Several libraries (ai, ext) build successfully
- ✅ **GLFW Migration**: GLFW is now the default graphics and event backend (complete integration)

### ❌ Remaining Work

- **Template Metaprogramming**: Some complex template code needs further C++17 updates
- **Smart Pointer Issues**: Hash table implementations need copy constructor fixes
- **Missing Includes**: Some files need additional standard library includes
- **Legacy Code Cleanup**: X11 and platform-specific driver code removal (not critical)

## GLFW Migration

The project has migrated from X11-based windowing to GLFW for cross-platform compatibility.

### Current Status
- ✅ **GLFW Dependency**: Added to CMake build system and linked properly
- ✅ **Basic Window Creation**: GLFW window and OpenGL context setup implemented
- ✅ **Event Handling Framework**: Basic GLFW event callbacks and input device classes
- ✅ **Error Handling**: GLFW error callback and basic logging
- ✅ **Default Backend**: GLFW is now the default graphics and event backend
- ✅ **Main Loop Integration**: GLFW integrates cleanly with existing game loop
- 🚧 **Legacy Code**: X11 code remains but is not used by default
- ❌ **Legacy Code Removal**: X11 and platform-specific code cleanup pending

### Files Added/Modified
- **New**: `src/hw/gfx_glfw.cpp` - GLFW-based graphics driver (now default)
- **New**: `src/hw/event_glfw.cpp` - GLFW-based event handling (now default)
- **Modified**: `CMakeLists.txt` - Added GLFW dependency and MONOLITHIC build mode
- **Modified**: `src/hw/CMakeLists.txt` - Updated to build GLFW modules and sound drivers
- **Modified**: `src/hw/gfx_drv.cpp` - Changed default driver from X11 to GLFW
- **Modified**: `src/hw/event_impl.cpp` - Changed default event system from X11 to GLFW
- **Legacy**: `src/hw/gfx_x11.cpp` and `src/hw/event_x11.cpp` - Available but not default

### GLFW Features Implemented
- Window creation with configurable resolution and fullscreen support
- OpenGL context setup with double buffering, alpha, and stencil buffers
- Basic event callbacks for keyboard, mouse, and window close events
- Desktop resolution detection and common video mode registration
- Proper GLFW initialization and cleanup
- Full integration with existing game main loop and event processing

## Architecture

The codebase is organized into modular libraries:

- `ai/` - Artificial intelligence and pathfinding
- `ext/` - External libraries (MP3 sound, OpenGL helpers)
- `game/` - Game logic and state management
- `gfx/` - Graphics rendering and management
- `hw/` - Hardware abstraction (sound, input, graphics)
- `main/` - Core math types and utilities
- `misc/` - Miscellaneous utilities (menus, parsing)
- `net/` - Network functionality
- `object/` - Game object management
- `phys/` - Physics simulation
- `res/` - Resource management
- `snd/` - Sound system
- `world/` - World/level management

## Original Game

Reaper3D was a 3D space combat game developed in the early 2000s. The original README describes it as:

> Enlisted as a mercenary pilot for a greedy corporation, you are to do whatever is deemed necessary in order to ensure profit...

### Controls
- Mouse/joystick/arrow keys for steering
- A/Z, 1-5, or Page Up/Down for thrust
- Q, End, or right mouse for boost
- M, joystick button 2, or middle mouse for missiles
- Space, Delete, or left mouse for laser

## Development Notes

This modernization effort brings a legacy early-2000s C++ codebase up to modern standards. The original code used pre-standard C++ constructs and extensive template metaprogramming that requires careful updating for C++17 compliance.

Key challenges addressed:
- Template-dependent name lookup strictness in C++17
- Deprecated STL constructs (auto_ptr, bind1st, not1)
- Exception specification syntax changes
- Smart pointer semantics updates
- Build system modernization

## Contributing

When contributing to the modernization effort:

1. Maintain minimal changes - preserve original logic where possible
2. Use modern C++17 idioms (auto, range-based for, smart pointers)
3. Fix template-dependent lookups with `this->` prefix
4. Replace deprecated STL constructs with modern equivalents
5. Add proper includes for standard library functions
6. Test incremental builds to avoid breaking existing functionality

## License

See the LICENSE file for license information.