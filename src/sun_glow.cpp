#include "circa-solem/sun_glow.hpp"

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

namespace cs {

namespace {
// Quad corner offsets: two triangles forming a unit square centered at origin.
// layout(location = 0) in vec2 aOffset
static constexpr float kQuadVerts[] = {
    -0.5f, -0.5f,
     0.5f, -0.5f,
     0.5f,  0.5f,
    -0.5f,  0.5f,
};
static constexpr unsigned int kQuadIdx[] = { 0, 1, 2,  0, 2, 3 };
} // namespace

SunGlow::SunGlow() {
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadVerts), kQuadVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kQuadIdx), kQuadIdx, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

SunGlow::~SunGlow() {
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
    glDeleteVertexArrays(1, &vao_);
}

void SunGlow::draw(const glm::mat4& view, const glm::mat4& proj,
                   const glm::vec3& sun_pos_world,
                   float sun_display_radius,
                   const ShaderProgram& shader) const {
    // Billboard half-size = 1.4× the Sun's rendered sphere radius.
    // This keeps the glow just outside the sphere surface at every zoom level.
    const float size = sun_display_radius * 1.4f;

    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.id(), "view"),       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.id(), "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3fv      (glGetUniformLocation(shader.id(), "uCenter"),    1, glm::value_ptr(sun_pos_world));
    glUniform1f       (glGetUniformLocation(shader.id(), "uSize"),      size);
    const glm::vec3 color{1.0f, 0.85f, 0.4f};
    glUniform3fv      (glGetUniformLocation(shader.id(), "uColor"),     1, glm::value_ptr(color));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // additive
    glDepthMask(GL_FALSE);              // don't let transparent corners corrupt depth buffer

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

} // namespace cs
