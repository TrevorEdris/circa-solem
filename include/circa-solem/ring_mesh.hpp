#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

namespace cs {

class ShaderProgram;

/// Textured annular disc rendered as a triangle strip in the XZ plane.
/// UV.x maps radial distance (0 = inner, 1 = outer) for ring density textures.
class RingMesh {
public:
    RingMesh(float inner_radius, float outer_radius, int segments = 128);
    ~RingMesh();

    RingMesh(const RingMesh&)            = delete;
    RingMesh& operator=(const RingMesh&) = delete;
    RingMesh(RingMesh&& o) noexcept;
    RingMesh& operator=(RingMesh&&) = delete;

    /// Draw the ring with a texture and color tint.
    /// Enables alpha blending and disables face culling for double-sided rendering.
    void draw(const glm::mat4& view, const glm::mat4& proj,
              const ShaderProgram& shader, const glm::mat4& model,
              GLuint texture, const glm::vec3& tint) const;

private:
    GLuint vao_        = 0;
    GLuint vbo_        = 0;
    int    vert_count_ = 0;
};

} // namespace cs
