#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

namespace cs {

/// Arcball orbit camera using spherical coordinates.
///
/// State: theta (azimuth), phi (elevation), radius, focus point.
/// - Left-click drag: rotate (delta theta/phi)
/// - Scroll: zoom (delta radius, clamped)
/// - Middle-click or Shift+left-drag: pan focus point
///
/// Attach once to a GLFW window via attachToWindow(); the camera installs
/// scroll and cursor callbacks. Key/button state is polled in update().
class Camera {
public:
    Camera();

    void attachToWindow(GLFWwindow* window);

    /// Poll input and update camera state. Call once per frame before view().
    void update(GLFWwindow* window, float delta_time);

    glm::mat4 view()               const;
    glm::mat4 projection(float aspect) const;
    glm::vec3 position()           const;

private:
    static void scroll_callback(GLFWwindow* w, double xoff, double yoff);

    float     theta_       = 0.3f;   // azimuth (radians)
    float     phi_         = 0.4f;   // elevation (radians)
    float     radius_      = 3.0f;   // AU
    glm::vec3 focus_       = {0.0f, 0.0f, 0.0f};

    // Mouse drag state
    double    last_x_      = 0.0;
    double    last_y_      = 0.0;
    bool      first_mouse_ = true;

    float     scroll_delta_ = 0.0f;  // accumulated between frames

    static constexpr float kRadiusMin = 0.01f;
    static constexpr float kRadiusMax = 200.0f;
    static constexpr float kPhiLimit  = 1.55f;  // ~89° — avoid gimbal lock
    static constexpr float kFov       = 45.0f;
};

} // namespace cs
