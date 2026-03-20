#pragma once

#include "circa-solem/shader_program.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

namespace cs {

/// Additive billboard quad rendered around the Sun to simulate a glow effect.
///
/// The quad is always screen-aligned (faces the camera). Size scales with
/// camera distance so the glow remains visually consistent across zoom levels.
/// Uses additive blending (GL_SRC_ALPHA, GL_ONE).
class SunGlow {
public:
    SunGlow();
    ~SunGlow();
    SunGlow(const SunGlow&)            = delete;
    SunGlow& operator=(const SunGlow&) = delete;

    /// sun_display_radius: the Sun's rendered sphere radius in world-space AU.
    /// The billboard is sized to 1.4× that radius so the glow wraps the sphere.
    void draw(const glm::mat4& view, const glm::mat4& proj,
              const glm::vec3& sun_pos_world,
              float sun_display_radius,
              const ShaderProgram& shader) const;

private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
};

} // namespace cs
