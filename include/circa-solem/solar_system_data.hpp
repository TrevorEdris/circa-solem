#pragma once

namespace cs::data {

// Physical constants for solar system bodies.
// Masses in solar masses; radii in km.
struct BodyData {
    const char* name;
    double      mass_msun;
    double      radius_km;
    float       r, g, b;
};

// ── Sun ─────────────────────────────────────────────────────────────────────

inline constexpr BodyData SUN     = {"Sun",     1.0,         695700.0, 1.00f, 0.95f, 0.70f};

// ── Inner planets ───────────────────────────────────────────────────────────

inline constexpr BodyData MERCURY = {"Mercury", 1.660e-7,      2439.7, 0.70f, 0.70f, 0.70f};
inline constexpr BodyData VENUS   = {"Venus",   2.448e-6,      6051.8, 0.95f, 0.90f, 0.70f};
inline constexpr BodyData EARTH   = {"Earth",   3.003e-6,      6371.0, 0.20f, 0.50f, 0.90f};
inline constexpr BodyData MARS    = {"Mars",    3.226e-7,      3389.5, 0.80f, 0.40f, 0.20f};
inline constexpr BodyData LUNA    = {"Moon",    3.693e-8,      1737.4, 0.75f, 0.75f, 0.75f};

// ── Outer planets ───────────────────────────────────────────────────────────

inline constexpr BodyData JUPITER = {"Jupiter", 9.548e-4,     69911.0, 0.85f, 0.75f, 0.55f};
inline constexpr BodyData SATURN  = {"Saturn",  2.858e-4,     58232.0, 0.90f, 0.82f, 0.60f};
inline constexpr BodyData URANUS  = {"Uranus",  4.366e-5,     25362.0, 0.65f, 0.85f, 0.90f};
inline constexpr BodyData NEPTUNE = {"Neptune", 5.151e-5,     24622.0, 0.30f, 0.45f, 0.90f};

// ── Dwarf planets ───────────────────────────────────────────────────────────

inline constexpr BodyData PLUTO   = {"Pluto",   6.583e-9,      1188.3, 0.75f, 0.70f, 0.65f};
inline constexpr BodyData CERES   = {"Ceres",   4.726e-10,      473.0, 0.65f, 0.65f, 0.65f};

// ── Galilean moons (Jupiter) ────────────────────────────────────────────────

inline constexpr BodyData IO       = {"Io",       4.492e-8,     1821.6, 0.95f, 0.90f, 0.40f};
inline constexpr BodyData EUROPA   = {"Europa",   2.413e-8,     1560.8, 0.85f, 0.80f, 0.70f};
inline constexpr BodyData GANYMEDE = {"Ganymede", 7.452e-8,     2634.1, 0.70f, 0.65f, 0.60f};
inline constexpr BodyData CALLISTO = {"Callisto", 5.412e-8,     2410.3, 0.45f, 0.40f, 0.35f};

// ── Saturnian moons ─────────────────────────────────────────────────────────

inline constexpr BodyData TITAN    = {"Titan",    6.768e-8,     2574.7, 0.85f, 0.75f, 0.45f};
inline constexpr BodyData RHEA     = {"Rhea",     1.162e-9,      763.8, 0.75f, 0.75f, 0.75f};

// ── Uranian moons ───────────────────────────────────────────────────────────

inline constexpr BodyData TITANIA  = {"Titania",  1.766e-9,      788.4, 0.70f, 0.70f, 0.75f};
inline constexpr BodyData OBERON   = {"Oberon",   1.515e-9,      761.4, 0.65f, 0.65f, 0.70f};

// ── Neptunian moons ─────────────────────────────────────────────────────────

inline constexpr BodyData TRITON   = {"Triton",   1.078e-8,     1353.4, 0.75f, 0.75f, 0.80f};

// ── Plutonian moons ─────────────────────────────────────────────────────────

inline constexpr BodyData CHARON   = {"Charon",   8.098e-10,     606.0, 0.65f, 0.60f, 0.60f};

} // namespace cs::data
