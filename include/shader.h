#ifndef SHADER_H
#define SHADER_H

#include <string>

#include <glad/gl.h>

class Shader
{
public:
    Shader();
    ~Shader();

    bool load_from_files(const std::string& vertex_path, const std::string& fragment_path);
    void use() const;
    void set_mat4(const char* name, const float* mat) const;
    void set_vec3(const char* name, float x, float y, float z) const;
    void set_int(const char* name, int value) const;
    void destroy();

private:
    GLuint compile_shader(GLenum type, const std::string& source);
    bool link_program(GLuint vs, GLuint fs);

public:
    GLuint program;
};

#endif
