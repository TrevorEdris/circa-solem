#pragma once

#include "circa-solem/shader_program.hpp"
#include <glm/glm.hpp>

namespace cs {

/// Renders a small XYZ axis indicator in the bottom-left corner of the viewport.
/// Red = X, Green = Y, Blue = Z.
///
/// Uses a rotation-only view matrix (strips camera translation) and an
/// orthographic projection, so the gizmo reflects camera orientation without
/// moving around the screen.
class AxisGizmo {
public:
    AxisGizmo();
    ~AxisGizmo();

    AxisGizmo(const AxisGizmo&)            = delete;
    AxisGizmo& operator=(const AxisGizmo&) = delete;

    /// Draw world-space XYZ axes at the origin.
    /// Uses the scene's own view/projection so no custom viewport is needed.
    /// Depth test is disabled so the axes are always visible.
    void draw(const glm::mat4& view, const glm::mat4& proj,
              const ShaderProgram& shader);

private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
};

} // namespace cs
