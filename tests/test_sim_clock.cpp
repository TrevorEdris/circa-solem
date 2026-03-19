#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "circa-solem/sim_clock.hpp"

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("julianDateFromCalendar — J2000 epoch", "[sim_clock]") {
    // J2000.0 = 2000-Jan-1 12:00 TT = JD 2451545.0
    double jd = cs::SimClock::julianDateFromCalendar(2000, 1, 1, 12, 0, 0);
    REQUIRE_THAT(jd, WithinAbs(2451545.0, 1e-6));
}

TEST_CASE("julianDateFromCalendar — Unix epoch", "[sim_clock]") {
    // Unix epoch = 1970-Jan-1 00:00:00 UTC = JD 2440587.5
    double jd = cs::SimClock::julianDateFromCalendar(1970, 1, 1, 0, 0, 0);
    REQUIRE_THAT(jd, WithinAbs(2440587.5, 1e-6));
}

TEST_CASE("setDate stores exact value", "[sim_clock]") {
    cs::SimClock clock;
    clock.setDate(2451545.0);
    REQUIRE_THAT(clock.julianDate(), WithinAbs(2451545.0, 1e-12));
}

TEST_CASE("advance increments by Julian years", "[sim_clock]") {
    cs::SimClock clock;
    clock.setDate(2451545.0);
    clock.advance(1.0);  // 1 Julian year = 365.25 days
    REQUIRE_THAT(clock.julianDate(), WithinAbs(2451545.0 + 365.25, 1e-9));
}

TEST_CASE("advance handles fractional years", "[sim_clock]") {
    cs::SimClock clock;
    clock.setDate(2451545.0);
    clock.advance(0.5);
    REQUIRE_THAT(clock.julianDate(), WithinAbs(2451545.0 + 182.625, 1e-9));
}

TEST_CASE("default-constructed clock is post-2023", "[sim_clock]") {
    // JD 2460000.0 = 2023-Feb-25; default clock should be at or after this
    cs::SimClock clock;
    REQUIRE(clock.julianDate() >= 2460000.0);
}
