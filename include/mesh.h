#ifndef MESH_H
#define MESH_H

#include <vector>

#include <glad/gl.h>

class Mesh
{
public:
    Mesh();
    ~Mesh();

    void upload_interleaved_positions_normals_and_indices(const std::vector<float>& vertices,
                                                          const std::vector<unsigned int>& indices);
    void draw() const;
    void destroy();

private:
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLsizei index_count;
};

#endif
