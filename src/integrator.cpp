#include "circa-solem/integrator.hpp"

#include <glm/geometric.hpp>
#include <algorithm>
#include <cmath>

namespace cs {

void Integrator::compute_accelerations(const std::vector<Body>& bodies,
                                        std::vector<glm::dvec3>& out_accel) const {
    const std::size_t n = bodies.size();
    out_accel.assign(n, glm::dvec3{0.0, 0.0, 0.0});

    for (std::size_t i = 0; i < n; ++i) {
        if (bodies[i].type != BodyType::SIMULATED) continue;

        for (std::size_t j = 0; j < n; ++j) {
            if (i == j || bodies[j].type != BodyType::SIMULATED) continue;

            const glm::dvec3 dr = bodies[j].position - bodies[i].position;
            const double r2 = glm::dot(dr, dr);
            const double r2_soft = std::max(r2, kEpsilon * kEpsilon);
            const double r_soft  = std::sqrt(r2_soft);

            // F = G * m_j / r_soft² * r̂ = G * m_j / r_soft³ * dr
            out_accel[i] += (kG * bodies[j].mass / (r2_soft * r_soft)) * dr;
        }
    }
}

void Integrator::step(std::vector<Body>& bodies, double dt) {
    const std::size_t n = bodies.size();

    // Velocity Verlet:
    //   a(t) already computed (or compute now)
    //   x(t+dt) = x(t) + v(t)*dt + 0.5*a(t)*dt²
    //   a(t+dt) = f(x(t+dt))
    //   v(t+dt) = v(t) + 0.5*(a(t) + a(t+dt))*dt

    std::vector<glm::dvec3> accel_t(n);
    compute_accelerations(bodies, accel_t);

    // Update positions
    for (std::size_t i = 0; i < n; ++i) {
        if (bodies[i].type != BodyType::SIMULATED) continue;
        bodies[i].position += bodies[i].velocity * dt
                            + 0.5 * accel_t[i] * (dt * dt);
    }

    // Compute new accelerations at updated positions
    std::vector<glm::dvec3> accel_t1(n);
    compute_accelerations(bodies, accel_t1);

    // Update velocities
    for (std::size_t i = 0; i < n; ++i) {
        if (bodies[i].type != BodyType::SIMULATED) continue;
        bodies[i].velocity += 0.5 * (accel_t[i] + accel_t1[i]) * dt;
    }
}

} // namespace cs
