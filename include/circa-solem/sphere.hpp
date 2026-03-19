#pragma once

#include <glad/gl.h>

namespace cs {

/// UV sphere geometry uploaded to GPU.
///
/// Vertex layout (interleaved, 32 bytes stride):
///   location 0: vec3 position
///   location 1: vec3 normal
///   location 2: vec2 uv
///
/// Generated as a unit sphere (radius 1). Scale via the model matrix.
class Sphere {
public:
    explicit Sphere(int lat_segs = 32, int lon_segs = 32);
    ~Sphere();

    Sphere(const Sphere&)            = delete;
    Sphere& operator=(const Sphere&) = delete;

    void draw() const;

    int indexCount() const { return index_count_; }

private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
    int    index_count_ = 0;
};

} // namespace cs
