/*
 * Modern GLSL shader support implementation
 */

#include "hw/compat.h"
#include "gfx/modern_shaders.h"
#include "hw/debug.h"
#include "res/res.h"
#include <fstream>
#include <sstream>

namespace reaper {
namespace gfx {
namespace modern_shaders {

namespace { debug::DebugOutput dout("gfx::modern_shaders"); }

// Built-in shader sources
namespace builtin {

const char* basic_vertex_shader = R"(
#version 120
attribute vec3 position;
attribute vec4 color;
varying vec4 vertexColor;

uniform mat4 mvpMatrix;

void main()
{
    gl_Position = mvpMatrix * vec4(position, 1.0);
    vertexColor = color;
}
)";

const char* basic_fragment_shader = R"(
#version 120
varying vec4 vertexColor;

void main()
{
    gl_FragColor = vertexColor;
}
)";

const char* texture_vertex_shader = R"(
#version 120
attribute vec3 position;
attribute vec2 texCoord;
varying vec2 fragTexCoord;

uniform mat4 mvpMatrix;

void main()
{
    gl_Position = mvpMatrix * vec4(position, 1.0);
    fragTexCoord = texCoord;
}
)";

const char* texture_fragment_shader = R"(
#version 120
uniform sampler2D tex;
varying vec2 fragTexCoord;

void main()
{
    gl_FragColor = texture2D(tex, fragTexCoord);
}
)";

} // namespace builtin

GLSLShader::GLSLShader(const std::string& shader_name)
    : program_id(0), vertex_shader(0), fragment_shader(0), name(shader_name), compiled(false)
{
}

GLSLShader::~GLSLShader()
{
    cleanup();
}

void GLSLShader::cleanup()
{
    // Shader functions not available in current OpenGL setup
    // This would clean up shaders in a full OpenGL 2.0+ environment
    compiled = false;
}

GLuint GLSLShader::compile_shader(const std::string& source, GLenum shader_type)
{
    // GLSL shaders not supported in current OpenGL setup
    dout << "GLSL shaders not available in this OpenGL context\n";
    return 0;
}

bool GLSLShader::check_compile_errors(GLuint shader, const std::string& type)
{
    // Not implemented for current OpenGL version
    return false;
}

bool GLSLShader::check_link_errors()
{
    // Not implemented for current OpenGL version
    return false;
}

bool GLSLShader::load_vertex_shader(const std::string& source)
{
    // Define GL_VERTEX_SHADER as 0 if not available
    const GLenum GL_VERTEX_SHADER_CONST = 0x8B31; // OpenGL 2.0 constant
    vertex_shader = compile_shader(source, GL_VERTEX_SHADER_CONST);
    return vertex_shader != 0;
}

bool GLSLShader::load_fragment_shader(const std::string& source)
{
    // Define GL_FRAGMENT_SHADER as 0 if not available  
    const GLenum GL_FRAGMENT_SHADER_CONST = 0x8B30; // OpenGL 2.0 constant
    fragment_shader = compile_shader(source, GL_FRAGMENT_SHADER_CONST);
    return fragment_shader != 0;
}

bool GLSLShader::load_shaders_from_files(const std::string& vertex_file, const std::string& fragment_file)
{
    // For now, use simple file reading - could integrate with resource system later
    std::ifstream vf(vertex_file);
    std::ifstream ff(fragment_file);
    
    if (!vf.is_open() || !ff.is_open()) {
        dout << "Failed to open shader files: " << vertex_file << ", " << fragment_file << "\n";
        return false;
    }
    
    std::stringstream vss, fss;
    vss << vf.rdbuf();
    fss << ff.rdbuf();
    
    return load_vertex_shader(vss.str()) && load_fragment_shader(fss.str());
}

bool GLSLShader::link()
{
    // GLSL not supported in current OpenGL setup
    dout << "GLSL shaders not supported, using fixed pipeline\n";
    return false;
}

void GLSLShader::use()
{
    // No-op when GLSL not supported
}

GLint GLSLShader::get_uniform_location(const std::string& name)
{
    return -1; // Not supported
}

void GLSLShader::set_uniform_float(const std::string& name, float value) { }
void GLSLShader::set_uniform_int(const std::string& name, int value) { }
void GLSLShader::set_uniform_vec3(const std::string& name, float x, float y, float z) { }
void GLSLShader::set_uniform_vec4(const std::string& name, float x, float y, float z, float w) { }
void GLSLShader::set_uniform_matrix4(const std::string& name, const float* matrix) { }

// ShaderManager implementation

ShaderManager::ShaderManager() : current_shader(nullptr)
{
}

ShaderManager::~ShaderManager()
{
    cleanup();
}

void ShaderManager::cleanup()
{
    for (auto& pair : shaders) {
        delete pair.second;
    }
    shaders.clear();
    current_shader = nullptr;
}

GLSLShader* ShaderManager::load_shader(const std::string& name,
                                      const std::string& vertex_file,
                                      const std::string& fragment_file)
{
    // Check if already loaded
    auto it = shaders.find(name);
    if (it != shaders.end()) {
        return it->second;
    }
    
    GLSLShader* shader = new GLSLShader(name);
    if (!shader->load_shaders_from_files(vertex_file, fragment_file) || !shader->link()) {
        delete shader;
        return nullptr;
    }
    
    shaders[name] = shader;
    return shader;
}

GLSLShader* ShaderManager::create_shader(const std::string& name,
                                        const std::string& vertex_source,
                                        const std::string& fragment_source)
{
    // Check if already exists
    auto it = shaders.find(name);
    if (it != shaders.end()) {
        return it->second;
    }
    
    GLSLShader* shader = new GLSLShader(name);
    if (!shader->load_vertex_shader(vertex_source) || 
        !shader->load_fragment_shader(fragment_source) || 
        !shader->link()) {
        delete shader;
        return nullptr;
    }
    
    shaders[name] = shader;
    return shader;
}

GLSLShader* ShaderManager::get_shader(const std::string& name)
{
    auto it = shaders.find(name);
    return (it != shaders.end()) ? it->second : nullptr;
}

bool ShaderManager::use_shader(const std::string& name)
{
    GLSLShader* shader = get_shader(name);
    if (shader && shader->is_ready()) {
        shader->use();
        current_shader = shader;
        return true;
    }
    return false;
}

void ShaderManager::use_fixed_pipeline()
{
    // Always use fixed pipeline in current OpenGL setup
    current_shader = nullptr;
}

bool ShaderManager::is_glsl_supported()
{
    // For current OpenGL setup, GLSL is not supported
    static bool checked = false;
    if (!checked) {
        dout << "GLSL support: NO (OpenGL < 2.0)\n";
        checked = true;
    }
    return false;
}

} // namespace modern_shaders
} // namespace gfx
} // namespace reaper