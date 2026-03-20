#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <utility>
#include <vector>

namespace cs {

class ShaderProgram;

/// Static particle field rendered as GL_POINTS.
/// Used for asteroid and Kuiper belts.
class ParticleField {
public:
    /// gaps: vector of {center_au, half_width_au} for Kirkwood-style gaps.
    ParticleField(int count, float inner_au, float outer_au,
                  float inclination_scatter_rad, unsigned seed,
                  const std::vector<std::pair<float, float>>& gaps = {});
    ~ParticleField();

    ParticleField(const ParticleField&)            = delete;
    ParticleField& operator=(const ParticleField&) = delete;
    ParticleField(ParticleField&& o) noexcept;
    ParticleField& operator=(ParticleField&&) = delete;

    void draw(const glm::mat4& view, const glm::mat4& proj,
              const ShaderProgram& shader, const glm::vec3& color) const;

private:
    GLuint vao_         = 0;
    GLuint vbo_         = 0;
    int    point_count_ = 0;
};

} // namespace cs
