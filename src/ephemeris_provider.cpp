#include "circa-solem/ephemeris_provider.hpp"
#include "circa-solem/constants.hpp"

#include <calceph.h>

#include <array>
#include <cmath>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace cs {

// Map satellite NAIF IDs to their parent planet barycenter NAIF ID.
// Returns 0 if the body is not a satellite (can be computed directly vs Sun).
static int parentBarycenter(int body_id) {
    if (body_id >= 301 && body_id <= 399) return 3;   // Earth-Moon barycenter
    if (body_id >= 501 && body_id <= 599) return 5;   // Jupiter barycenter
    if (body_id >= 601 && body_id <= 699) return 6;   // Saturn barycenter
    if (body_id >= 701 && body_id <= 799) return 7;   // Uranus barycenter
    if (body_id >= 801 && body_id <= 899) return 8;   // Neptune barycenter
    if (body_id >= 901 && body_id <= 999) return 9;   // Pluto barycenter
    return 0;
}

EphemerisProvider::EphemerisProvider(const std::string& ephemeris_dir) {
    std::vector<std::string> paths;
    for (const auto& entry : std::filesystem::directory_iterator(ephemeris_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".bsp") {
            paths.push_back(entry.path().string());
        }
    }

    if (paths.empty()) {
        throw std::runtime_error(
            "EphemerisProvider: no .bsp files found in: " + ephemeris_dir);
    }

    std::vector<const char*> cstrs;
    cstrs.reserve(paths.size());
    for (const auto& p : paths) {
        cstrs.push_back(p.c_str());
    }

    pbd_ = calceph_open_array(static_cast<int>(cstrs.size()), cstrs.data());
    if (!pbd_) {
        throw std::runtime_error(
            "EphemerisProvider: calceph_open_array failed for " +
            std::to_string(paths.size()) + " files in: " + ephemeris_dir);
    }
}

EphemerisProvider::~EphemerisProvider() {
    if (pbd_) calceph_close(static_cast<t_calcephbin*>(pbd_));
}

// Raw calceph call: compute PV of target relative to center, returned in km and km/day.
static bool computeRaw(void* pbd, double JD0, double time,
                       int target, int center, std::array<double, 6>& PV) {
    constexpr int kUnits = CALCEPH_UNIT_KM | CALCEPH_UNIT_DAY | CALCEPH_USE_NAIFID;
    return calceph_compute_unit(static_cast<t_calcephbin*>(pbd),
                                JD0, time, target, center, kUnits,
                                PV.data()) != 0;
}

EphemerisProvider::StateVector
EphemerisProvider::getStateVector(int body_id, double julian_date) const {
    const double JD0  = static_cast<double>(static_cast<long long>(julian_date));
    const double time = julian_date - JD0;

    std::array<double, 6> PV{};

    // Try direct computation (target relative to Sun).
    if (!computeRaw(pbd_, JD0, time, body_id, SUN, PV)) {
        // Direct failed — try two-step via parent barycenter for satellites.
        const int parent = parentBarycenter(body_id);
        if (parent == 0) {
            throw std::runtime_error(
                "EphemerisProvider: calceph_compute_unit failed for body " +
                std::to_string(body_id));
        }

        // Step 1: satellite relative to parent barycenter
        std::array<double, 6> PV_sat{};
        if (!computeRaw(pbd_, JD0, time, body_id, parent, PV_sat)) {
            throw std::runtime_error(
                "EphemerisProvider: failed satellite→parent for body " +
                std::to_string(body_id) + " center " + std::to_string(parent));
        }

        // Step 2: parent barycenter relative to Sun
        std::array<double, 6> PV_parent{};
        if (!computeRaw(pbd_, JD0, time, parent, SUN, PV_parent)) {
            throw std::runtime_error(
                "EphemerisProvider: failed parent→Sun for barycenter " +
                std::to_string(parent));
        }

        // Combine: satellite relative to Sun = parent_sun + sat_parent
        for (int i = 0; i < 6; ++i) {
            PV[i] = PV_parent[i] + PV_sat[i];
        }
    }

    // km → AU;  km/day → AU/yr
    const double ps = 1.0 / kKmPerAU;
    const double vs = kDaysPerYr / kKmPerAU;
    const glm::dvec3 pos_icrf{ PV[0] * ps, PV[1] * ps, PV[2] * ps };
    const glm::dvec3 vel_icrf{ PV[3] * vs, PV[4] * vs, PV[5] * vs };

    // Rotate ICRF equatorial → ecliptic J2000: Rx(-ε)
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
