#pragma once

#include <ctime>

namespace cs {

/// Tracks simulation time as a Julian Date (JD).
///
/// Julian Date is a continuous count of days from noon on January 1, 4713 BC.
/// J2000.0 = JD 2451545.0 = 2000-Jan-1 12:00 TT
///
/// Time unit used by the integrator: Julian years (365.25 days).
/// advance(dt_years) adds dt_years * 365.25 to the stored JD.
///
/// Note: julianDateFromCalendar() uses the proleptic Gregorian calendar and
/// is accurate for dates roughly within J2000 ± 10,000 years. Edge cases
/// near the Gregorian reform (Oct 1582) are out of scope for this project.
class SimClock {
public:
    /// Default-construct to the current wall-clock date (approximate UTC→JD).
    SimClock();

    /// Set the current Julian Date directly.
    void setDate(double jd);

    /// Advance time by dt_years Julian years (365.25 days each).
    void advance(double dt_years);

    /// Return the current Julian Date.
    double julianDate() const;

    /// Convert a calendar date/time to Julian Date.
    /// hour, min, sec are in UTC (no leap-second correction).
    static double julianDateFromCalendar(int year, int month, int day,
                                         int hour, int min, int sec);

private:
    double jd_;
};

} // namespace cs
