#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "circa-solem/ephemeris_provider.hpp"

#include <glm/glm.hpp>
#include <cstdlib>
#include <cmath>
#include <stdexcept>

using Catch::Matchers::WithinAbs;

// EPHEMERIS_DIR is injected by CMake via target_compile_definitions
static std::string ephemeris_dir() {
    const char* env = std::getenv("EPHEMERIS_DIR");
    if (env) return env;
    return EPHEMERIS_DIR;
}

// ── Inner planets (existing tests) ──────────────────────────────────────────

TEST_CASE("EphemerisProvider — Earth distance at J2000 ≈ 0.983 AU", "[ephemeris]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    auto sv = ep.getStateVector(cs::EphemerisProvider::EARTH, 2451545.0);
    double r = glm::length(sv.position_au);
    REQUIRE_THAT(r, WithinAbs(0.9833, 0.01));
}

TEST_CASE("EphemerisProvider — Earth stays near ecliptic plane (Y ≈ 0)", "[ephemeris]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    auto sv = ep.getStateVector(cs::EphemerisProvider::EARTH, 2451545.0);
    REQUIRE(std::abs(sv.position_au.y) < 0.01);
}

TEST_CASE("EphemerisProvider — Sun relative to itself is near origin", "[ephemeris]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    auto sv = ep.getStateVector(cs::EphemerisProvider::SUN, 2451545.0);
    REQUIRE(glm::length(sv.position_au) < 0.001);
}

TEST_CASE("EphemerisProvider — Earth position at 2010-Jan-1 within 0.01 AU of Horizons", "[ephemeris]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    auto sv = ep.getStateVector(cs::EphemerisProvider::EARTH, 2455197.5);
    REQUIRE_THAT(sv.position_au.x, WithinAbs(-0.1743, 0.01));
    REQUIRE_THAT(sv.position_au.y, WithinAbs( 0.0007, 0.01));
    REQUIRE_THAT(sv.position_au.z, WithinAbs(-0.9669, 0.01));
}

TEST_CASE("EphemerisProvider — Moon is within 0.003 AU of Earth", "[ephemeris]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    auto earth = ep.getStateVector(cs::EphemerisProvider::EARTH, 2451545.0);
    auto moon  = ep.getStateVector(cs::EphemerisProvider::MOON,  2451545.0);
    double dist = glm::length(moon.position_au - earth.position_au);
    REQUIRE(dist > 0.001);
    REQUIRE(dist < 0.003);
}

TEST_CASE("EphemerisProvider — invalid body ID throws runtime_error", "[ephemeris]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    REQUIRE_THROWS_AS(ep.getStateVector(9999, 2451545.0), std::runtime_error);
}

TEST_CASE("EphemerisProvider — Earth velocity at J2000 ≈ 6.28 AU/yr", "[ephemeris]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    auto sv = ep.getStateVector(cs::EphemerisProvider::EARTH, 2451545.0);
    double speed = glm::length(sv.velocity_au_yr);
    REQUIRE_THAT(speed, WithinAbs(6.283, 0.2));
}

// ── Outer planets (Phase 3A) ────────────────────────────────────────────────

TEST_CASE("EphemerisProvider — Jupiter distance ≈ 4.9-5.5 AU at J2000", "[ephemeris][outer]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    auto sv = ep.getStateVector(cs::EphemerisProvider::JUPITER, 2451545.0);
    double r = glm::length(sv.position_au);
    REQUIRE(r > 4.9);
    REQUIRE(r < 5.5);
}

TEST_CASE("EphemerisProvider — Saturn distance ≈ 8.9-10.1 AU at J2000", "[ephemeris][outer]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    auto sv = ep.getStateVector(cs::EphemerisProvider::SATURN, 2451545.0);
    double r = glm::length(sv.position_au);
    REQUIRE(r > 8.9);
    REQUIRE(r < 10.1);
}

TEST_CASE("EphemerisProvider — Uranus distance ≈ 18.5-20.5 AU at J2000", "[ephemeris][outer]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    auto sv = ep.getStateVector(cs::EphemerisProvider::URANUS, 2451545.0);
    double r = glm::length(sv.position_au);
    REQUIRE(r > 18.5);
    REQUIRE(r < 20.5);
}

TEST_CASE("EphemerisProvider — Neptune distance ≈ 29.5-30.5 AU at J2000", "[ephemeris][outer]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    auto sv = ep.getStateVector(cs::EphemerisProvider::NEPTUNE, 2451545.0);
    double r = glm::length(sv.position_au);
    REQUIRE(r > 29.5);
    REQUIRE(r < 30.5);
}

TEST_CASE("EphemerisProvider — Pluto distance ≈ 29-34 AU at J2000", "[ephemeris][outer]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    auto sv = ep.getStateVector(cs::EphemerisProvider::PLUTO, 2451545.0);
    double r = glm::length(sv.position_au);
    REQUIRE(r > 29.0);
    REQUIRE(r < 34.0);
}

// ── Tier 1 moon proximity tests ─────────────────────────────────────────────

TEST_CASE("EphemerisProvider — Io is within 0.01 AU of Jupiter", "[ephemeris][moons]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    auto jup = ep.getStateVector(cs::EphemerisProvider::JUPITER, 2451545.0);
    auto io  = ep.getStateVector(cs::EphemerisProvider::IO,      2451545.0);
    double dist = glm::length(io.position_au - jup.position_au);
    REQUIRE(dist < 0.01);
}

TEST_CASE("EphemerisProvider — Titan is within 0.01 AU of Saturn", "[ephemeris][moons]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    auto sat   = ep.getStateVector(cs::EphemerisProvider::SATURN, 2451545.0);
    auto titan = ep.getStateVector(cs::EphemerisProvider::TITAN,  2451545.0);
    double dist = glm::length(titan.position_au - sat.position_au);
    REQUIRE(dist < 0.01);
}

TEST_CASE("EphemerisProvider — Triton is within 0.01 AU of Neptune", "[ephemeris][moons]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    auto nep    = ep.getStateVector(cs::EphemerisProvider::NEPTUNE, 2451545.0);
    auto triton = ep.getStateVector(cs::EphemerisProvider::TRITON,  2451545.0);
    double dist = glm::length(triton.position_au - nep.position_au);
    REQUIRE(dist < 0.01);
}

TEST_CASE("EphemerisProvider — Charon is within 0.001 AU of Pluto", "[ephemeris][moons]") {
    cs::EphemerisProvider ep(ephemeris_dir());
    auto plu    = ep.getStateVector(cs::EphemerisProvider::PLUTO,  2451545.0);
    auto charon = ep.getStateVector(cs::EphemerisProvider::CHARON, 2451545.0);
    double dist = glm::length(charon.position_au - plu.position_au);
    REQUIRE(dist < 0.001);
}
