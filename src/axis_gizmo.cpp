#include "circa-solem/axis_gizmo.hpp"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace cs {

// 6 vertices: one line per axis (origin → tip), interleaved pos(xyz) + color(rgb).
// Axes are 1 unit long in local space; caller scales via model matrix.
// X = red, Y = green, Z = blue.
static const float kAxisVerts[] = {
    // pos            color
    0.f, 0.f, 0.f,   1.f, 0.2f, 0.2f,  // X origin
    1.f, 0.f, 0.f,   1.f, 0.2f, 0.2f,  // X tip
    0.f, 0.f, 0.f,   0.2f, 1.f, 0.2f,  // Y origin
    0.f, 1.f, 0.f,   0.2f, 1.f, 0.2f,  // Y tip
    0.f, 0.f, 0.f,   0.2f, 0.4f, 1.f,  // Z origin
    0.f, 0.f, 1.f,   0.2f, 0.4f, 1.f,  // Z tip
};

AxisGizmo::AxisGizmo() {
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kAxisVerts), kAxisVerts, GL_STATIC_DRAW);

    constexpr GLsizei stride = 6 * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));

    glBindVertexArray(0);
}

AxisGizmo::~AxisGizmo() {
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
}

void AxisGizmo::draw(const glm::mat4& view, const glm::mat4& proj,
                     const ShaderProgram& shader)
{
    glDisable(GL_DEPTH_TEST);

    // Scale axes to 0.15 AU — visible from the default 3 AU camera distance,
    // large enough to see clearly, small enough not to obscure the Sun sphere.
    const glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.15f));
    const glm::mat4 mvp   = proj * view * model;

    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.id(), "mvp"), 1, GL_FALSE,
                       glm::value_ptr(mvp));

    glBindVertexArray(vao_);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

} // namespace cs
