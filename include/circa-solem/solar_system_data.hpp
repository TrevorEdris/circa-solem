#pragma once

namespace cs::data {

// Physical constants for inner solar system bodies.
// Masses in solar masses; radii in km.
struct BodyData {
    const char* name;
    double      mass_msun;
    double      radius_km;
    float       r, g, b;
};

inline constexpr BodyData SUN     = {"Sun",     1.0,         695700.0, 1.00f, 0.95f, 0.70f};
inline constexpr BodyData MERCURY = {"Mercury", 1.660e-7,      2439.7, 0.70f, 0.70f, 0.70f};
inline constexpr BodyData VENUS   = {"Venus",   2.448e-6,      6051.8, 0.95f, 0.90f, 0.70f};
inline constexpr BodyData EARTH   = {"Earth",   3.003e-6,      6371.0, 0.20f, 0.50f, 0.90f};
inline constexpr BodyData MARS    = {"Mars",    3.226e-7,      3389.5, 0.80f, 0.40f, 0.20f};
inline constexpr BodyData LUNA    = {"Moon",    3.693e-8,      1737.4, 0.75f, 0.75f, 0.75f};

} // namespace cs::data
