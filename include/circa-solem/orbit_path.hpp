#pragma once

#include "circa-solem/shader_program.hpp"
#include <glm/glm.hpp>

namespace cs {

/// Renders a circular orbit path as a GL_LINE_LOOP in the XZ plane.
class OrbitPath {
public:
    /// radius_au: orbit radius in AU. segments: number of line segments.
    explicit OrbitPath(float radius_au, int segments = 360);
    ~OrbitPath();

    OrbitPath(const OrbitPath&)            = delete;
    OrbitPath& operator=(const OrbitPath&) = delete;
    OrbitPath(OrbitPath&& o) noexcept
        : vao_(o.vao_), vbo_(o.vbo_), segments_(o.segments_)
    { o.vao_ = 0; o.vbo_ = 0; }

    /// model: optional transform to tilt the ring to its orbital plane.
    void draw(const glm::mat4& view, const glm::mat4& proj,
              const ShaderProgram& shader,
              const glm::mat4& model = glm::mat4{1.0f}) const;

private:
    GLuint vao_      = 0;
    GLuint vbo_      = 0;
    int    segments_ = 0;
};

} // namespace cs
