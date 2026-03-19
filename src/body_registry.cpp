#include "circa-solem/body_registry.hpp"

#include <algorithm>

namespace cs {

void BodyRegistry::add(Body body) {
    bodies_.push_back(std::move(body));
}

void BodyRegistry::remove(std::string_view name) {
    bodies_.erase(
        std::remove_if(bodies_.begin(), bodies_.end(),
                       [&](const Body& b) { return b.name == name; }),
        bodies_.end());
}

const std::vector<Body>& BodyRegistry::bodies() const { return bodies_; }
std::vector<Body>&       BodyRegistry::bodies()       { return bodies_; }

} // namespace cs
