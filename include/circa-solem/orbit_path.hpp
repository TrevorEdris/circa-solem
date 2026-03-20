#pragma once

#include "circa-solem/shader_program.hpp"
#include <glm/glm.hpp>

namespace cs {

/// Renders an elliptical orbit path as a GL_LINE_LOOP in the XZ plane.
class OrbitPath {
public:
    /// semi_major_axis: in AU. eccentricity: [0, 1). omega_rad: argument of
    /// periapsis (rotation of perihelion within the orbital plane).
    OrbitPath(float semi_major_axis, float eccentricity, float omega_rad,
             int segments = 360);
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
