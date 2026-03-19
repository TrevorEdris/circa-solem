#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "circa-solem/ephemeris_provider.hpp"

#include <glm/glm.hpp>
#include <cstdlib>
#include <cmath>
#include <stdexcept>

using Catch::Matchers::WithinAbs;

// DE440_PATH is injected by CMake via target_compile_definitions
static std::string de440_path() {
    const char* env = std::getenv("DE440_PATH");
    if (env) return env;
    return DE440_PATH;
}

TEST_CASE("EphemerisProvider — Earth distance at J2000 ≈ 0.983 AU", "[ephemeris]") {
    cs::EphemerisProvider ep(de440_path());
    // J2000.0 = JD 2451545.0 (2000-Jan-1 12:00 TT)
    auto sv = ep.getStateVector(cs::EphemerisProvider::EARTH, 2451545.0);
    double r = glm::length(sv.position_au);
    // Earth is near perihelion in early January; distance ≈ 0.9833 AU
    REQUIRE_THAT(r, WithinAbs(0.9833, 0.01));
}

TEST_CASE("EphemerisProvider — Earth stays near ecliptic plane (Y ≈ 0)", "[ephemeris]") {
    cs::EphemerisProvider ep(de440_path());
    auto sv = ep.getStateVector(cs::EphemerisProvider::EARTH, 2451545.0);
    // Y is the out-of-ecliptic axis in Y-up convention; Earth should be near zero
    REQUIRE(std::abs(sv.position_au.y) < 0.01);
}

TEST_CASE("EphemerisProvider — Sun relative to itself is near origin", "[ephemeris]") {
    cs::EphemerisProvider ep(de440_path());
    auto sv = ep.getStateVector(cs::EphemerisProvider::SUN, 2451545.0);
    REQUIRE(glm::length(sv.position_au) < 0.001);
}

TEST_CASE("EphemerisProvider — Earth position at 2010-Jan-1 within 0.01 AU of Horizons", "[ephemeris]") {
    cs::EphemerisProvider ep(de440_path());
    // JD 2455197.5 = 2010-Jan-1 00:00 TT
    // JPL Horizons heliocentric ecliptic J2000 (AU): X=-0.1743, Y=0.9669, Z=0.0007
    // Y-up transform {eclX, eclZ, -eclY} → our frame: X=-0.1743, Y=0.0007, Z=-0.9669
    auto sv = ep.getStateVector(cs::EphemerisProvider::EARTH, 2455197.5);
    REQUIRE_THAT(sv.position_au.x, WithinAbs(-0.1743, 0.01));   // ecliptic X
    REQUIRE_THAT(sv.position_au.y, WithinAbs( 0.0007, 0.01));   // ecliptic Z (out-of-plane, ~0)
    REQUIRE_THAT(sv.position_au.z, WithinAbs(-0.9669, 0.01));   // -ecliptic Y
}

TEST_CASE("EphemerisProvider — Moon is within 0.003 AU of Earth", "[ephemeris]") {
    cs::EphemerisProvider ep(de440_path());
    auto earth = ep.getStateVector(cs::EphemerisProvider::EARTH, 2451545.0);
    auto moon  = ep.getStateVector(cs::EphemerisProvider::MOON,  2451545.0);
    // Moon orbital radius ≈ 384,400 km ≈ 0.00257 AU; allow ±0.003 AU tolerance
    double dist = glm::length(moon.position_au - earth.position_au);
    REQUIRE(dist > 0.001);  // Moon is not at Earth's center
    REQUIRE(dist < 0.003);  // Moon is within 0.003 AU of Earth
}

TEST_CASE("EphemerisProvider — invalid body ID throws runtime_error", "[ephemeris]") {
    cs::EphemerisProvider ep(de440_path());
    // NAIF ID 9999 does not exist in DE440; calceph_compute_unit should return 0.
    REQUIRE_THROWS_AS(ep.getStateVector(9999, 2451545.0), std::runtime_error);
}

TEST_CASE("EphemerisProvider — Earth velocity at J2000 ≈ 6.28 AU/yr", "[ephemeris]") {
    cs::EphemerisProvider ep(de440_path());
    // Earth mean orbital speed: 2π AU / 1 yr ≈ 6.283 AU/yr
    auto sv = ep.getStateVector(cs::EphemerisProvider::EARTH, 2451545.0);
    double speed = glm::length(sv.velocity_au_yr);
    REQUIRE_THAT(speed, WithinAbs(6.283, 0.2));
}
