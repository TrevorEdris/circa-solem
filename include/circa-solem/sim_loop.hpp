#pragma once

#include "circa-solem/body_registry.hpp"
#include "circa-solem/integrator.hpp"
#include "circa-solem/sim_clock.hpp"

namespace cs {

/// Drives the physics simulation at a fixed substep size.
///
/// Decouples physics rate from render rate:
///   - Physics advances in fixed substeps of kSubstepYears (1 minute).
///   - tick(wall_dt_seconds) is called once per rendered frame.
///   - warp_factor scales wall-clock time to simulation time.
///
/// Default warp: 50,000× real-time ≈ Earth orbit in ~10 minutes.
class SimLoop {
public:
    explicit SimLoop(BodyRegistry& registry, SimClock& clock,
                     double warp_factor = 50000.0);

    /// Advance the simulation by wall_dt_seconds of wall-clock time.
    void tick(double wall_dt_seconds);

    double warpFactor() const;
    void   setWarpFactor(double w);

private:
    BodyRegistry& registry_;
    SimClock&     clock_;
    Integrator    integrator_;
    double        warp_factor_;
    double        accumulated_years_ = 0.0;  // carry-over from last tick

    // Fixed physics substep: one minute.
    // Must be smaller than sim_years/frame at the maximum expected warp so
    // the while loop fires every frame and motion appears smooth.
    // At 50,000× warp, 60 fps: ~14 steps/frame with a 1-minute substep.
    static constexpr double kSubstepYears     = 1.0 / (365.25 * 24.0 * 60.0);
    static constexpr double kWallSecondsPerYear = 365.25 * 86400.0;
};

} // namespace cs
