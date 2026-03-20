#include "circa-solem/ring_mesh.hpp"
#include "circa-solem/shader_program.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <vector>

namespace cs {

RingMesh::RingMesh(float inner_radius, float outer_radius, int segments) {
    // Triangle strip: alternating inner/outer vertices around the ring.
    // Each segment has 2 vertices; close the strip by repeating the first pair.
    const int vert_count = (segments + 1) * 2;
    vert_count_ = vert_count;

    struct Vertex {
        glm::vec3 pos;
        glm::vec2 uv;
    };

    std::vector<Vertex> verts;
    verts.reserve(static_cast<std::size_t>(vert_count));

    const float pi2 = 2.0f * static_cast<float>(M_PI);
    for (int i = 0; i <= segments; ++i) {
        const float angle = pi2 * static_cast<float>(i) / static_cast<float>(segments);
        const float ca = std::cos(angle);
        const float sa = std::sin(angle);

        // Inner vertex (UV.x = 0)
        verts.push_back({{inner_radius * ca, 0.0f, inner_radius * sa},
                         {0.0f, static_cast<float>(i) / static_cast<float>(segments)}});
        // Outer vertex (UV.x = 1)
        verts.push_back({{outer_radius * ca, 0.0f, outer_radius * sa},
                         {1.0f, static_cast<float>(i) / static_cast<float>(segments)}});
    }

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(verts.size() * sizeof(Vertex)),
                 verts.data(), GL_STATIC_DRAW);

    // position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, pos)));
    // uv (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, uv)));

    glBindVertexArray(0);
}

RingMesh::~RingMesh() {
    if (vao_) {
        glDeleteBuffers(1, &vbo_);
        glDeleteVertexArrays(1, &vao_);
    }
}

RingMesh::RingMesh(RingMesh&& o) noexcept
    : vao_(o.vao_), vbo_(o.vbo_), vert_count_(o.vert_count_)
{
    o.vao_ = 0;
    o.vbo_ = 0;
}

void RingMesh::draw(const glm::mat4& view, const glm::mat4& proj,
                    const ShaderProgram& shader, const glm::mat4& model,
                    GLuint texture, const glm::vec3& tint) const
{
    // Save GL state
    GLboolean blend_was_on = GL_FALSE;
    glGetBooleanv(GL_BLEND, &blend_was_on);
    GLboolean cull_was_on = GL_FALSE;
    glGetBooleanv(GL_CULL_FACE, &cull_was_on);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.id(), "model"),      1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader.id(), "view"),       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.id(), "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3fv(glGetUniformLocation(shader.id(), "tint_color"), 1, glm::value_ptr(tint));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shader.id(), "ring_texture"), 0);

    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vert_count_);
    glBindVertexArray(0);

    // Restore GL state
    if (!blend_was_on) glDisable(GL_BLEND);
    if (cull_was_on)   glEnable(GL_CULL_FACE);
}

} // namespace cs
