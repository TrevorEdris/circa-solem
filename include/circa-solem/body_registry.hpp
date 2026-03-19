#pragma once

#include "circa-solem/body.hpp"

#include <string_view>
#include <vector>

namespace cs {

/// Owns the collection of all bodies in the simulation.
/// The Integrator operates on the full vector and filters by BodyType internally.
class BodyRegistry {
public:
    void add(Body body);

    /// Remove by name. No-op if name not found.
    void remove(std::string_view name);

    const std::vector<Body>& bodies() const;
    std::vector<Body>&       bodies();

private:
    std::vector<Body> bodies_;
};

} // namespace cs
