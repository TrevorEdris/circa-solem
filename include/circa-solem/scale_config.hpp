#pragma once

namespace cs {

/// Render-only scale factors — physics always uses raw AU values.
struct ScaleConfig {
    float size_scale     = 1000.0f;  // multiply physical radius (in AU) for rendering
    float distance_scale = 1.0f;     // multiply orbital positions (1.0 = true distances)

    static ScaleConfig display()    { return {1000.0f, 1.0f}; }
    static ScaleConfig true_scale() { return {1.0f,    1.0f}; }
};

} // namespace cs
