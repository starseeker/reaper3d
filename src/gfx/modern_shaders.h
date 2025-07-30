/*
 * Modern GLSL shader support for OpenGL 2.0+
 * Provides GLSL vertex and fragment shader compilation and management
 */

#ifndef REAPER_GFX_MODERN_SHADERS_H
#define REAPER_GFX_MODERN_SHADERS_H

#include "hw/gl.h"
#include <string>
#include <map>

namespace reaper {
namespace gfx {
namespace modern_shaders {

// GLSL Shader wrapper class
class GLSLShader {
private:
    GLuint program_id;
    GLuint vertex_shader;
    GLuint fragment_shader;
    std::string name;
    bool compiled;
    
public:
    GLSLShader(const std::string& shader_name);
    ~GLSLShader();
    
    // Load and compile shaders from source
    bool load_vertex_shader(const std::string& source);
    bool load_fragment_shader(const std::string& source);
    bool load_shaders_from_files(const std::string& vertex_file, const std::string& fragment_file);
    
    // Link the program
    bool link();
    
    // Use this shader program
    void use();
    
    // Get uniform locations
    GLint get_uniform_location(const std::string& name);
    
    // Set uniform values
    void set_uniform_float(const std::string& name, float value);
    void set_uniform_int(const std::string& name, int value);
    void set_uniform_vec3(const std::string& name, float x, float y, float z);
    void set_uniform_vec4(const std::string& name, float x, float y, float z, float w);
    void set_uniform_matrix4(const std::string& name, const float* matrix);
    
    // Check if shader is ready to use
    bool is_ready() const { return compiled && program_id != 0; }
    
private:
    GLuint compile_shader(const std::string& source, GLenum shader_type);
    bool check_compile_errors(GLuint shader, const std::string& type);
    bool check_link_errors();
    void cleanup();
};

// Shader manager for loading and caching shaders
class ShaderManager {
private:
    std::map<std::string, GLSLShader*> shaders;
    GLSLShader* current_shader;
    
public:
    ShaderManager();
    ~ShaderManager();
    
    // Load a shader from files
    GLSLShader* load_shader(const std::string& name, 
                           const std::string& vertex_file, 
                           const std::string& fragment_file);
    
    // Load a shader from source strings
    GLSLShader* create_shader(const std::string& name,
                             const std::string& vertex_source,
                             const std::string& fragment_source);
    
    // Get a loaded shader
    GLSLShader* get_shader(const std::string& name);
    
    // Use a shader by name
    bool use_shader(const std::string& name);
    
    // Stop using any shader (return to fixed pipeline)
    void use_fixed_pipeline();
    
    // Get currently active shader
    GLSLShader* get_current_shader() { return current_shader; }
    
    // Check if GLSL is supported
    static bool is_glsl_supported();
    
private:
    void cleanup();
};

// Simple built-in shaders for common tasks
namespace builtin {
    // Basic vertex + fragment shader for colored vertices
    extern const char* basic_vertex_shader;
    extern const char* basic_fragment_shader;
    
    // Simple texture shader
    extern const char* texture_vertex_shader;
    extern const char* texture_fragment_shader;
}

} // namespace modern_shaders
} // namespace gfx
} // namespace reaper

#endif // REAPER_GFX_MODERN_SHADERS_H