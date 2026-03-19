#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "circa-solem/body.hpp"
#include "circa-solem/integrator.hpp"

#include <glm/glm.hpp>
#include <cmath>
#include <vector>

using Catch::Matchers::WithinRel;

// Unit system: AU, solar masses, Julian years.
// G = 4π² in these units (Earth at 1 AU has period 1 yr with circular velocity 2π AU/yr).

static constexpr double G_AU    = 4.0 * M_PI * M_PI;
static constexpr double M_SUN   = 1.0;
static constexpr double M_EARTH = 3.003e-6;  // solar masses

static cs::Body make_sun() {
    cs::Body b;
    b.name     = "Sun";
    b.mass     = M_SUN;
    b.position = {0.0, 0.0, 0.0};
    b.velocity = {0.0, 0.0, 0.0};
    b.type     = cs::BodyType::SIMULATED;
    return b;
}

static cs::Body make_earth_circular() {
    // Circular orbit at 1 AU: v = sqrt(G * M_sun / r) = sqrt(4π²) = 2π AU/yr
    cs::Body b;
    b.name     = "Earth";
    b.mass     = M_EARTH;
    b.position = {1.0, 0.0, 0.0};
    b.velocity = {0.0, 2.0 * M_PI, 0.0};
    b.type     = cs::BodyType::SIMULATED;
    return b;
}

static double total_energy(const std::vector<cs::Body>& bodies) {
    double ke = 0.0;
    for (const auto& b : bodies) {
        double v2 = glm::dot(b.velocity, b.velocity);
        ke += 0.5 * b.mass * v2;
    }
    double pe = 0.0;
    for (std::size_t i = 0; i < bodies.size(); ++i) {
        for (std::size_t j = i + 1; j < bodies.size(); ++j) {
            glm::dvec3 dr = bodies[j].position - bodies[i].position;
            double r = glm::length(dr);
            pe -= G_AU * bodies[i].mass * bodies[j].mass / r;
        }
    }
    return ke + pe;
}

TEST_CASE("Velocity Verlet — energy conservation over 1000 steps", "[integrator]") {
    std::vector<cs::Body> bodies = {make_sun(), make_earth_circular()};
    cs::Integrator integrator;

    const double dt = 1.0 / 365.0;  // one day in Julian years
    const double E0 = total_energy(bodies);

    for (int i = 0; i < 1000; ++i) {
        integrator.step(bodies, dt);
    }

    const double E1 = total_energy(bodies);
    // Energy should be conserved within 0.1%
    REQUIRE_THAT(E1, WithinRel(E0, 0.001));
}

TEST_CASE("Velocity Verlet — Earth completes ~1 orbit in 365 steps", "[integrator]") {
    std::vector<cs::Body> bodies = {make_sun(), make_earth_circular()};
    cs::Integrator integrator;

    const double dt = 1.0 / 365.0;
    const glm::dvec3 start_pos = bodies[1].position;

    for (int i = 0; i < 365; ++i) {
        integrator.step(bodies, dt);
    }

    // Angle swept ≈ 2π: dot(norm(start), norm(end)) should be ≈ 1.0
    // (within ~2° = cos(2°) ≈ 0.9994)
    const glm::dvec3 end_pos = bodies[1].position;
    const double cos_angle = glm::dot(glm::normalize(start_pos), glm::normalize(end_pos));
    REQUIRE(cos_angle > 0.9994);
}

TEST_CASE("Decorative bodies are not integrated", "[integrator]") {
    std::vector<cs::Body> bodies;

    cs::Body sun = make_sun();
    bodies.push_back(sun);

    cs::Body deco;
    deco.name     = "Decorative";
    deco.mass     = 1.0;
    deco.position = {5.0, 0.0, 0.0};
    deco.velocity = {0.0, 0.0, 0.0};
    deco.type     = cs::BodyType::DECORATIVE;
    bodies.push_back(deco);

    cs::Integrator integrator;
    integrator.step(bodies, 1.0 / 365.0);

    // Decorative body must not move
    REQUIRE(bodies[1].position.x == 5.0);
    REQUIRE(bodies[1].position.y == 0.0);
    REQUIRE(bodies[1].velocity.x == 0.0);
}
