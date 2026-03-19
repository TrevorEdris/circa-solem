#pragma once

#include "circa-solem/body.hpp"

#include <glm/glm.hpp>
#include <vector>

namespace cs {

/// Velocity Verlet N-body gravitational integrator.
///
/// Unit system: AU (distance), solar masses (mass), Julian years (time).
/// G = 4π² in these units.
///
/// Only bodies with BodyType::SIMULATED participate in the force calculation.
/// DECORATIVE bodies are left untouched by step().
///
/// Softening parameter ε = 1e-4 AU prevents singularity at near-zero separation.
class Integrator {
public:
    void step(std::vector<Body>& bodies, double dt);

private:
    /// Compute gravitational acceleration on each SIMULATED body.
    void compute_accelerations(const std::vector<Body>& bodies,
                               std::vector<glm::dvec3>& out_accel) const;

    static constexpr double kG       = 4.0 * 3.14159265358979323846 * 3.14159265358979323846;
    static constexpr double kEpsilon = 1e-4;  // AU — softening length
};

} // namespace cs
