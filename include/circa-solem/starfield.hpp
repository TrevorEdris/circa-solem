#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

namespace cs {

class ShaderProgram;

/// Procedural starfield: ~5000 random GL_POINTS on a large sphere.
///
/// Rendered with depth writes disabled and gl_Position.z = gl_Position.w
/// (far-plane trick), so stars are always behind all scene geometry
/// regardless of world-space radius or the active scale system.
///
/// Requires glEnable(GL_VERTEX_PROGRAM_POINT_SIZE) before first draw
/// (called once in main).
class Starfield {
public:
    explicit Starfield(int count = 5000, float radius = 10000.0f, unsigned int seed = 42);
    ~Starfield();

    Starfield(const Starfield&)            = delete;
    Starfield& operator=(const Starfield&) = delete;

    void draw(const glm::mat4& view, const glm::mat4& proj,
              const ShaderProgram& shader) const;

private:
    GLuint vao_        = 0;
    GLuint vbo_        = 0;
    int    point_count_ = 0;
};

} // namespace cs
