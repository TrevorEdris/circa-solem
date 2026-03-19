#pragma once

#include <glm/glm.hpp>
#include <string>

namespace cs {

/// Wraps the calceph library to provide body state vectors from JPL DE440 .bsp files.
///
/// Positions are in AU, ecliptic J2000, Y-up convention (orbit plane is XZ).
/// Velocities are in AU/Julian year.
///
/// Coordinate chain (applied inside getStateVector):
///   ICRF equatorial J2000 → rotate around X by +ε (obliquity 23.44°)
///   → ecliptic J2000 (XY orbit, Z-up) → { x, z, -y } → Y-up XZ orbit
class EphemerisProvider {
public:
    explicit EphemerisProvider(const std::string& de440_path);
    ~EphemerisProvider();

    EphemerisProvider(const EphemerisProvider&) = delete;
    EphemerisProvider& operator=(const EphemerisProvider&) = delete;

    struct StateVector {
        glm::dvec3 position_au;    // AU, ecliptic J2000 Y-up (orbit in XZ plane)
        glm::dvec3 velocity_au_yr; // AU/Julian year
    };

    /// State vector of body relative to the Sun at the given Julian Date (TDB).
    /// body_id must be a NAIF ID (see constants below).
    StateVector getStateVector(int body_id, double julian_date) const;

    // NAIF body IDs — standard SPICE/Horizons numbering
    static constexpr int MERCURY = 199;
    static constexpr int VENUS   = 299;
    static constexpr int EARTH   = 399;
    static constexpr int MARS    = 499;
    static constexpr int MOON    = 301;
    static constexpr int SUN     = 10;

private:
    void* pbd_ = nullptr;  // opaque calceph t_calcephbin* handle

    // Obliquity of ecliptic at J2000 (IAU value, radians)
    static constexpr double kObliquity = 23.439291 * (3.14159265358979323846 / 180.0);
    static constexpr double kDaysPerYr = 365.25;
};

} // namespace cs
