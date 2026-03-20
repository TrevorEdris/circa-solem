#include "circa-solem/orbit_trail.hpp"

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <numeric>
#include <vector>

namespace cs {

OrbitTrail::OrbitTrail(int capacity)
    : capacity_(capacity)
    , positions_(capacity)
    , scratch_(capacity)
{
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_pos_);
    glGenBuffers(1, &vbo_alpha_);

    // Pre-compute alpha values: oldest = 0.0, newest = 0.6
    std::vector<float> alphas(capacity);
    for (int i = 0; i < capacity; ++i) {
        alphas[i] = 0.6f * static_cast<float>(i) / static_cast<float>(capacity - 1);
    }

    glBindVertexArray(vao_);

    // Position buffer (location 0) — contents updated in push()
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_);
    glBufferData(GL_ARRAY_BUFFER,
                 capacity * static_cast<GLsizei>(sizeof(glm::vec3)),
                 nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(0);

    // Alpha buffer (location 1) — static, computed once
    glBindBuffer(GL_ARRAY_BUFFER, vbo_alpha_);
    glBufferData(GL_ARRAY_BUFFER,
                 capacity * static_cast<GLsizei>(sizeof(float)),
                 alphas.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), nullptr);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

OrbitTrail::~OrbitTrail() {
    glDeleteBuffers(1, &vbo_pos_);
    glDeleteBuffers(1, &vbo_alpha_);
    glDeleteVertexArrays(1, &vao_);
}

void OrbitTrail::push(const glm::dvec3& world_pos) {
    positions_[head_] = glm::vec3(world_pos);
    head_ = (head_ + 1) % capacity_;
    if (count_ < capacity_) ++count_;
    upload_gpu();
}

void OrbitTrail::upload_gpu() {
    // Reorder the ring buffer into a contiguous chronological array for drawing.
    // Oldest is at head_ when the buffer is full (count_ == capacity_).
    const int oldest = (count_ < capacity_) ? 0 : head_;

    for (int i = 0; i < count_; ++i) {
        scratch_[i] = positions_[(oldest + i) % capacity_];
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    count_ * static_cast<GLsizei>(sizeof(glm::vec3)),
                    scratch_.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OrbitTrail::draw(const glm::mat4& view, const glm::mat4& proj,
                      const ShaderProgram& shader,
                      const glm::vec3& color) const {
    if (count_ < 2) return;

    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.id(), "view"),       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.id(), "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3fv      (glGetUniformLocation(shader.id(), "uColor"),     1, glm::value_ptr(color));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(vao_);
    glDrawArrays(GL_LINE_STRIP, 0, count_);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
}

void OrbitTrail::clear() {
    head_  = 0;
    count_ = 0;
}

} // namespace cs
