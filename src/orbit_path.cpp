#include "circa-solem/orbit_path.hpp"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <vector>

namespace cs {

OrbitPath::OrbitPath(float semi_major_axis, float eccentricity, float omega_rad,
                     int segments)
    : segments_(segments)
{
    std::vector<float> verts;
    verts.reserve(segments * 6);  // pos(xyz) + color(rgb) per vertex

    const float a  = semi_major_axis;
    const float e  = eccentricity;
    const float b  = a * std::sqrt(1.0f - e * e);   // semi-minor axis
    const float c  = a * e;                           // focus offset from center
    const float co = std::cos(omega_rad);
    const float so = std::sin(omega_rad);
    const float pi = static_cast<float>(M_PI);

    for (int i = 0; i < segments; ++i) {
        // Parametric eccentric anomaly E ∈ [0, 2π)
        const float E = 2.0f * pi * static_cast<float>(i) / static_cast<float>(segments);

        // Ellipse relative to focus (Sun at origin), periapsis along +X before rotation
        const float xl = a * std::cos(E) - c;   // local X (along periapsis)
        const float zl = b * std::sin(E);        // local Z (perpendicular)

        // Rotate by argument of periapsis ω around Y axis
        verts.push_back(xl * co - zl * so);  // X
        verts.push_back(0.0f);               // Y (orbit in XZ plane)
        verts.push_back(xl * so + zl * co);  // Z
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
                     const ShaderProgram& shader,
                     const glm::mat4& model) const
{
    const glm::mat4 mvp = proj * view * model;

    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.id(), "mvp"), 1, GL_FALSE,
                       glm::value_ptr(mvp));

    glBindVertexArray(vao_);
    glDrawArrays(GL_LINE_LOOP, 0, segments_);
    glBindVertexArray(0);
}

} // namespace cs
