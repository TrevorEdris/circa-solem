#include "circa-solem/particle_field.hpp"
#include "circa-solem/shader_program.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <random>

namespace cs {

ParticleField::ParticleField(int count, float inner_au, float outer_au,
                             float inclination_scatter_rad, unsigned seed,
                             const std::vector<std::pair<float, float>>& gaps)
{
    std::mt19937 rng{seed};
    std::uniform_real_distribution<float> angle_dist(
        0.0f, 2.0f * static_cast<float>(M_PI));
    std::uniform_real_distribution<float> radius_dist(inner_au, outer_au);
    std::uniform_real_distribution<float> incl_dist(
        -inclination_scatter_rad, inclination_scatter_rad);
    std::uniform_real_distribution<float> brightness_dist(0.3f, 0.7f);
    std::uniform_real_distribution<float> size_dist(1.0f, 2.5f);

    struct ParticleVertex {
        glm::vec3 position;
        float     brightness;
        float     point_size;
    };

    std::vector<ParticleVertex> particles;
    particles.reserve(static_cast<std::size_t>(count));

    auto in_gap = [&](float r) -> bool {
        for (const auto& [center, half_w] : gaps) {
            if (std::abs(r - center) < half_w) return true;
        }
        return false;
    };

    int attempts = 0;
    while (static_cast<int>(particles.size()) < count && attempts < count * 10) {
        ++attempts;
        const float r = radius_dist(rng);
        if (in_gap(r)) continue;

        const float theta = angle_dist(rng);
        const float y_off = r * std::sin(incl_dist(rng));

        ParticleVertex p;
        p.position   = {r * std::cos(theta), y_off, r * std::sin(theta)};
        p.brightness = brightness_dist(rng);
        p.point_size = size_dist(rng);
        particles.push_back(p);
    }

    point_count_ = static_cast<int>(particles.size());

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(particles.size() * sizeof(ParticleVertex)),
                 particles.data(), GL_STATIC_DRAW);

    // position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex),
                          reinterpret_cast<void*>(offsetof(ParticleVertex, position)));
    // brightness (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex),
                          reinterpret_cast<void*>(offsetof(ParticleVertex, brightness)));
    // point_size (location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex),
                          reinterpret_cast<void*>(offsetof(ParticleVertex, point_size)));

    glBindVertexArray(0);
}

ParticleField::~ParticleField() {
    if (vao_) {
        glDeleteBuffers(1, &vbo_);
        glDeleteVertexArrays(1, &vao_);
    }
}

ParticleField::ParticleField(ParticleField&& o) noexcept
    : vao_(o.vao_), vbo_(o.vbo_), point_count_(o.point_count_)
{
    o.vao_ = 0;
    o.vbo_ = 0;
}

void ParticleField::draw(const glm::mat4& view, const glm::mat4& proj,
                         const ShaderProgram& shader,
                         const glm::vec3& color) const
{
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.id(), "view"),       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.id(), "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3fv(glGetUniformLocation(shader.id(), "particle_color"), 1, glm::value_ptr(color));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(vao_);
    glDrawArrays(GL_POINTS, 0, point_count_);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
}

} // namespace cs
