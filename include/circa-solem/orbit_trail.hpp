#pragma once

#include "circa-solem/shader_program.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

namespace cs {

/// Ring buffer of recent body positions drawn as a fading GL_LINE_STRIP.
///
/// Push a position each simulation step (or every N steps). The oldest positions
/// fade to transparent; the newest appear at full opacity. Toggle visibility
/// with the 'O' key in main.cpp.
class OrbitTrail {
public:
    explicit OrbitTrail(int capacity = 500);
    ~OrbitTrail();
    OrbitTrail(const OrbitTrail&)            = delete;
    OrbitTrail& operator=(const OrbitTrail&) = delete;

    void push(const glm::dvec3& world_pos);
    void draw(const glm::mat4& view, const glm::mat4& proj,
              const ShaderProgram& shader, const glm::vec3& color) const;
    void clear();

private:
    int              capacity_;
    int              head_  = 0;
    int              count_ = 0;
    std::vector<glm::vec3> positions_;

    GLuint vao_       = 0;
    GLuint vbo_pos_   = 0;
    GLuint vbo_alpha_ = 0;

    void upload_gpu();
};

} // namespace cs
