#include "circa-solem/ephemeris_provider.hpp"

#include <calceph.h>

#include <array>
#include <cmath>
#include <stdexcept>

namespace cs {

EphemerisProvider::EphemerisProvider(const std::string& de440_path) {
    pbd_ = calceph_open(de440_path.c_str());
    if (!pbd_) {
        throw std::runtime_error(
            "EphemerisProvider: failed to open DE440 at: " + de440_path);
    }
}

EphemerisProvider::~EphemerisProvider() {
    if (pbd_) calceph_close(static_cast<t_calcephbin*>(pbd_));
}

EphemerisProvider::StateVector
EphemerisProvider::getStateVector(int body_id, double julian_date) const {
    // calceph splits JD into integer + fractional parts for precision.
    const double JD0  = static_cast<double>(static_cast<long long>(julian_date));
    const double time = julian_date - JD0;

    // CALCEPH_UNIT_KM | CALCEPH_UNIT_DAY → positions in km, velocities in km/day.
    // DE440 .bsp files don't embed an AU constant, so AU units are unavailable;
    // we convert manually using the IAU 2012 definition below.
    // CALCEPH_USE_NAIFID → target/center use NAIF IDs (Earth=399, Sun=10, etc.).
    constexpr int kUnits = CALCEPH_UNIT_KM | CALCEPH_UNIT_DAY | CALCEPH_USE_NAIFID;
    constexpr double kKmPerAU = 1.495978707e8;  // km/AU (IAU 2012)

    std::array<double, 6> PV{};
    const int rc = calceph_compute_unit(static_cast<t_calcephbin*>(pbd_),
                                        JD0, time, body_id, SUN, kUnits,
                                        PV.data());
    if (rc == 0) {
        throw std::runtime_error(
            "EphemerisProvider: calceph_compute_unit failed for body " +
            std::to_string(body_id));
    }

    // km → AU;  km/day → AU/yr
    const double ps = 1.0 / kKmPerAU;
    const double vs = kDaysPerYr / kKmPerAU;
    const glm::dvec3 pos_icrf{ PV[0] * ps, PV[1] * ps, PV[2] * ps };
    const glm::dvec3 vel_icrf{ PV[3] * vs, PV[4] * vs, PV[5] * vs };

    // Rotate ICRF equatorial → ecliptic J2000: Rx(-ε)
    // (positive ε tilts equatorial Z toward ecliptic -Y, so we rotate by -ε)
    const double c = std::cos(kObliquity);
    const double s = std::sin(kObliquity);
    auto rx = [c, s](const glm::dvec3& v) -> glm::dvec3 {
        return { v.x,  c * v.y + s * v.z,  -s * v.y + c * v.z };
    };

    const glm::dvec3 pos_ecl = rx(pos_icrf);
    const glm::dvec3 vel_ecl = rx(vel_icrf);

    // Ecliptic J2000 (XY orbit, Z-up) → Y-up XZ orbit: { x, z, -y }
    return {
        { pos_ecl.x,  pos_ecl.z, -pos_ecl.y },
        { vel_ecl.x,  vel_ecl.z, -vel_ecl.y }
    };
}

} // namespace cs
