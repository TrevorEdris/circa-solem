#include "circa-solem/sim_loop.hpp"

#include <algorithm>

namespace cs {

SimLoop::SimLoop(BodyRegistry& registry, SimClock& clock, double warp_factor)
    : registry_(registry)
    , clock_(clock)
    , warp_factor_(warp_factor)
{}

void SimLoop::tick(double wall_dt_seconds) {
    // Convert wall-clock delta to simulation years, scaled by warp.
    // Cap to 120 substeps worth of accumulated time to prevent a runaway
    // integration loop after a debugger pause, window drag, or slow first frame.
    const double sim_years = (wall_dt_seconds / kWallSecondsPerYear) * warp_factor_;
    accumulated_years_ = std::min(accumulated_years_ + sim_years, kSubstepYears * 120.0);

    // Advance in fixed substeps to maintain integrator accuracy.
    while (accumulated_years_ >= kSubstepYears) {
        integrator_.step(registry_.bodies(), kSubstepYears);
        clock_.advance(kSubstepYears);
        accumulated_years_ -= kSubstepYears;
    }
}

double SimLoop::warpFactor() const { return warp_factor_; }
void   SimLoop::setWarpFactor(double w) { warp_factor_ = w; }

} // namespace cs
