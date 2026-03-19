#include "circa-solem/starfield.hpp"
#include "circa-solem/shader_program.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <random>
#include <vector>
#include <cmath>

namespace cs {

namespace {
    // Uniform sphere sampling (Marsaglia 1972)
    glm::vec3 random_on_sphere(std::mt19937& rng) {
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        glm::vec3 v;
        float len2;
        do {
            v    = {dist(rng), dist(rng), dist(rng)};
            len2 = glm::dot(v, v);
        } while (len2 >= 1.0f || len2 < 1e-8f);
        return v / std::sqrt(len2);
    }
} // namespace

Starfield::Starfield(int count, float radius, unsigned int seed) {
    std::mt19937 rng{seed};
    std::uniform_real_distribution<float> brightness_dist(0.4f, 1.0f);
    std::uniform_real_distribution<float> size_dist(1.0f, 3.0f);

    struct StarVertex {
        glm::vec3 position;
        float     brightness;
        float     point_size;
    };

    std::vector<StarVertex> stars;
    stars.reserve(static_cast<std::size_t>(count));

    for (int i = 0; i < count; ++i) {
        StarVertex s;
        s.position   = random_on_sphere(rng) * radius;
        s.brightness = brightness_dist(rng);
        s.point_size = size_dist(rng);
        stars.push_back(s);
    }

    point_count_ = count;

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(stars.size() * sizeof(StarVertex)),
                 stars.data(), GL_STATIC_DRAW);

    // position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(StarVertex),
                          reinterpret_cast<void*>(offsetof(StarVertex, position)));
    // brightness (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(StarVertex),
                          reinterpret_cast<void*>(offsetof(StarVertex, brightness)));
    // point_size (location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(StarVertex),
                          reinterpret_cast<void*>(offsetof(StarVertex, point_size)));

    glBindVertexArray(0);
}

Starfield::~Starfield() {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
}

void Starfield::draw(const glm::mat4& view, const glm::mat4& proj,
                      const ShaderProgram& shader) const {
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.id(), "view"),       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.id(), "projection"), 1, GL_FALSE, glm::value_ptr(proj));

    glDepthMask(GL_FALSE);
    glBindVertexArray(vao_);
    glDrawArrays(GL_POINTS, 0, point_count_);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
}

} // namespace cs
