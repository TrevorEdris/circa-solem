#pragma once

#include <numbers>

namespace cs {

/// Kilometres per astronomical unit (IAU 2012 nominal value).
inline constexpr double kKmPerAU = 1.495978707e8;

// ── Planet axial tilts (obliquity to ecliptic, radians) ──────────────────────

inline constexpr float kSaturnObliquity  = 26.73f  * std::numbers::pi_v<float> / 180.0f;
inline constexpr float kUranusObliquity  = 97.77f  * std::numbers::pi_v<float> / 180.0f;
inline constexpr float kNeptuneObliquity = 28.32f  * std::numbers::pi_v<float> / 180.0f;

// ── Ring dimensions (km) ─────────────────────────────────────────────────────

// Saturn: D ring inner edge to F ring outer edge
inline constexpr double kSaturnRingInnerKm  =  66900.0;
inline constexpr double kSaturnRingOuterKm  = 136775.0;

// Uranus: innermost (6) to outermost (epsilon) ring
inline constexpr double kUranusRingInnerKm  =  41837.0;
inline constexpr double kUranusRingOuterKm  =  51149.0;

// Neptune: Galle ring inner to Adams ring outer
inline constexpr double kNeptuneRingInnerKm =  41900.0;
inline constexpr double kNeptuneRingOuterKm =  62932.0;

} // namespace cs
