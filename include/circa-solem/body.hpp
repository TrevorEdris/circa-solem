#pragma once

#include <glm/glm.hpp>
#include <string>

namespace cs {

enum class BodyType {
    SIMULATED,   // gravitationally interacts via N-body integration
    DECORATIVE,  // moves on a prescribed path, not gravitationally integrated
};

struct Body {
    std::string  name;
    double       mass       = 0.0;    // solar masses
    double       radius_km  = 0.0;    // physical radius in km
    glm::dvec3   position   = {};     // AU
    glm::dvec3   velocity   = {};     // AU / Julian year
    glm::vec3    color      = {1.0f, 1.0f, 1.0f};
    std::string  parent;       // empty = orbits Sun; non-empty = name of parent body
    BodyType     type       = BodyType::SIMULATED;
};

} // namespace cs
