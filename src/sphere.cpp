#include "circa-solem/sphere.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <cmath>

namespace cs {

Sphere::Sphere(int lat_segs, int lon_segs) {
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    std::vector<Vertex>        verts;
    std::vector<unsigned int>  indices;

    verts.reserve(static_cast<std::size_t>((lat_segs + 1) * (lon_segs + 1)));

    for (int lat = 0; lat <= lat_segs; ++lat) {
        const float theta     = static_cast<float>(lat) / static_cast<float>(lat_segs) * glm::pi<float>();
        const float sin_theta = std::sin(theta);
        const float cos_theta = std::cos(theta);

        for (int lon = 0; lon <= lon_segs; ++lon) {
            const float phi     = static_cast<float>(lon) / static_cast<float>(lon_segs) * glm::two_pi<float>();
            const float sin_phi = std::sin(phi);
            const float cos_phi = std::cos(phi);

            glm::vec3 n = {sin_theta * cos_phi, cos_theta, sin_theta * sin_phi};
            verts.push_back({n, n, {static_cast<float>(lon) / lon_segs,
                                     static_cast<float>(lat) / lat_segs}});
        }
    }

    for (int lat = 0; lat < lat_segs; ++lat) {
        for (int lon = 0; lon < lon_segs; ++lon) {
            const unsigned int a = static_cast<unsigned int>(lat * (lon_segs + 1) + lon);
            const unsigned int b = a + static_cast<unsigned int>(lon_segs + 1);
            indices.push_back(a);     indices.push_back(b);     indices.push_back(a + 1);
            indices.push_back(a + 1); indices.push_back(b);     indices.push_back(b + 1);
        }
    }

    index_count_ = static_cast<int>(indices.size());

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(verts.size() * sizeof(Vertex)),
                 verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                 indices.data(), GL_STATIC_DRAW);

    // position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, pos)));
    // normal (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, normal)));
    // uv (location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, uv)));

    glBindVertexArray(0);
}

Sphere::~Sphere() {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
}

void Sphere::draw() const {
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

} // namespace cs
