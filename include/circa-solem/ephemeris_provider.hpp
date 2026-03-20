#pragma once

#include <glm/glm.hpp>
#include <string>

namespace cs {

/// Reads JPL SPICE .bsp ephemeris files via calceph and evaluates body state
/// vectors (position + velocity) at any Julian Date.
///
/// Loads ALL .bsp files found in the given directory via calceph_open_array,
/// enabling automatic chain evaluation (e.g., Sun→Jupiter barycenter from DE440
/// combined with Jupiter barycenter→Io from jup365.bsp).
///
/// Positions are in AU, ecliptic J2000, Y-up convention (orbit plane is XZ).
/// Velocities are in AU/Julian year.
class EphemerisProvider {
public:
    explicit EphemerisProvider(const std::string& ephemeris_dir);
    ~EphemerisProvider();

    EphemerisProvider(const EphemerisProvider&)            = delete;
    EphemerisProvider& operator=(const EphemerisProvider&) = delete;
    EphemerisProvider(EphemerisProvider&&)                 = delete;
    EphemerisProvider& operator=(EphemerisProvider&&)      = delete;

    struct StateVector {
        glm::dvec3 position_au;
        glm::dvec3 velocity_au_yr;
    };

    /// State vector of body relative to the Sun at the given Julian Date (TDB).
    StateVector getStateVector(int body_id, double julian_date) const;

    // ── NAIF Body IDs ────────────────────────────────────────────────────────

    static constexpr int SUN     = 10;

    // Inner planets
    static constexpr int MERCURY = 199;
    static constexpr int VENUS   = 299;
    static constexpr int EARTH   = 399;
    static constexpr int MARS    = 4;     // Mars Barycenter (DE440 has 4, not 499)
    static constexpr int MOON    = 301;

    // Outer planet barycenters (DE440)
    static constexpr int JUPITER = 5;
    static constexpr int SATURN  = 6;
    static constexpr int URANUS  = 7;
    static constexpr int NEPTUNE = 8;
    static constexpr int PLUTO   = 9;

    // Galilean moons (jup365.bsp)
    static constexpr int IO       = 501;
    static constexpr int EUROPA   = 502;
    static constexpr int GANYMEDE = 503;
    static constexpr int CALLISTO = 504;

    // Saturnian moons (sat441.bsp)
    static constexpr int RHEA  = 605;
    static constexpr int TITAN = 606;

    // Uranian moons (ura116xl.bsp)
    static constexpr int TITANIA = 703;
    static constexpr int OBERON  = 704;

    // Neptunian moons (nep097.bsp)
    static constexpr int TRITON = 801;

    // Plutonian moons (plu060.bsp)
    static constexpr int CHARON = 901;

    // Dwarf planets / small bodies (sb441-n16.bsp)
    static constexpr int CERES = 2000001;

private:
    void* pbd_ = nullptr;

    static constexpr double kObliquity = 23.439291 * (3.14159265358979323846 / 180.0);
    static constexpr double kDaysPerYr = 365.25;
};

} // namespace cs
