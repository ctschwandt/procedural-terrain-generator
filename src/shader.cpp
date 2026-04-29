#include "shader.h"

#include <iostream>

#include "file_utils.h"

Shader::Shader()
    : program(0)
{
}

Shader::~Shader()
{
    destroy();
}

GLuint Shader::compile_shader(GLenum type, const std::string& source)
{
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char info[4096];
        glGetShaderInfoLog(shader, sizeof(info), nullptr, info);
        std::cerr << "Shader compile error:\n" << info << "\n";
    }

    return shader;
}

bool Shader::link_program(GLuint vs, GLuint fs)
{
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char info[4096];
        glGetProgramInfoLog(program, sizeof(info), nullptr, info);
        std::cerr << "Program link error:\n" << info << "\n";
        return false;
    }

    return true;
}

bool Shader::load_from_files(const std::string& vertex_path, const std::string& fragment_path)
{
    destroy();

    std::string vs_text;
    std::string fs_text;

    if (!read_text_file(vertex_path, vs_text))
    {
        std::cerr << "Failed reading vertex shader: " << vertex_path << "\n";
        return false;
    }

    if (!read_text_file(fragment_path, fs_text))
    {
        std::cerr << "Failed reading fragment shader: " << fragment_path << "\n";
        return false;
    }

    GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_text);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_text);

    bool ok = link_program(vs, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return ok;
}

void Shader::use() const
{
    glUseProgram(program);
}

void Shader::set_mat4(const char* name, const float* mat) const
{
    GLint loc = glGetUniformLocation(program, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, mat);
}

void Shader::set_vec3(const char* name, float x, float y, float z) const
{
    GLint loc = glGetUniformLocation(program, name);
    glUniform3f(loc, x, y, z);
}

void Shader::set_int(const char* name, int value) const
{
    GLint loc = glGetUniformLocation(program, name);
    glUniform1i(loc, value);
}

void Shader::destroy()
{
    if (program != 0)
    {
        glDeleteProgram(program);
        program = 0;
    }
}
