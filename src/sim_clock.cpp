#include "circa-solem/sim_clock.hpp"

namespace cs {

namespace {
    constexpr double kJulianYearDays = 365.25;
    // Unix epoch (1970-Jan-1 00:00 UTC) in Julian Date
    constexpr double kUnixEpochJD    = 2440587.5;
    constexpr double kSecondsPerDay  = 86400.0;
} // namespace

SimClock::SimClock() {
    const std::time_t now = std::time(nullptr);
    jd_ = kUnixEpochJD + static_cast<double>(now) / kSecondsPerDay;
}

void SimClock::setDate(double jd) {
    jd_ = jd;
}

void SimClock::advance(double dt_years) {
    jd_ += dt_years * kJulianYearDays;
}

double SimClock::julianDate() const {
    return jd_;
}

// static
double SimClock::julianDateFromCalendar(int year, int month, int day,
                                         int hour, int min, int sec) {
    // Standard algorithm for Gregorian calendar → Julian Day Number.
    // Reference: Meeus, "Astronomical Algorithms", Chapter 7.
    const int a = (14 - month) / 12;
    const int y = year + 4800 - a;
    const int m = month + 12 * a - 3;

    const int jdn = day + (153 * m + 2) / 5 + 365 * y + y / 4
                  - y / 100 + y / 400 - 32045;

    const double fractional_day = (static_cast<double>(hour) - 12.0) / 24.0
                                + static_cast<double>(min)  / 1440.0
                                + static_cast<double>(sec)  / 86400.0;

    return static_cast<double>(jdn) + fractional_day;
}

} // namespace cs
