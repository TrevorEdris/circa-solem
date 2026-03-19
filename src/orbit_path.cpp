#include "circa-solem/orbit_path.hpp"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <vector>

namespace cs {

OrbitPath::OrbitPath(float radius_au, int segments)
    : segments_(segments)
{
    std::vector<float> verts;
    verts.reserve(segments * 6);  // pos(xyz) + color(rgb) per vertex

    const float r  = radius_au;
    const float pi = static_cast<float>(M_PI);

    for (int i = 0; i < segments; ++i) {
        const float angle = 2.0f * pi * static_cast<float>(i) / static_cast<float>(segments);
        verts.push_back(r * std::cos(angle));  // X
        verts.push_back(0.0f);                 // Y (orbit in XZ plane)
        verts.push_back(r * std::sin(angle));  // Z
        // Dim white — visible but not distracting
        verts.push_back(0.4f);
        verts.push_back(0.4f);
        verts.push_back(0.4f);
    }

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(verts.size() * sizeof(float)),
                 verts.data(), GL_STATIC_DRAW);

    constexpr GLsizei stride = 6 * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(3 * sizeof(float)));

    glBindVertexArray(0);
}

OrbitPath::~OrbitPath() {
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
}

void OrbitPath::draw(const glm::mat4& view, const glm::mat4& proj,
                     const ShaderProgram& shader) const
{
    const glm::mat4 mvp = proj * view;

    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.id(), "mvp"), 1, GL_FALSE,
                       glm::value_ptr(mvp));

    glBindVertexArray(vao_);
    glDrawArrays(GL_LINE_LOOP, 0, segments_);
    glBindVertexArray(0);
}

} // namespace cs
