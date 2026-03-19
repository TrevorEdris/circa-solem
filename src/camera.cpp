#include "circa-solem/camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace cs {

// GLFW user-pointer key for the Camera instance.
static constexpr const char* kCameraKey = "camera";

Camera::Camera() = default;

void Camera::attachToWindow(GLFWwindow* window) {
    glfwSetWindowUserPointer(window, this);
    glfwSetScrollCallback(window, scroll_callback);
}

// static
void Camera::scroll_callback(GLFWwindow* w, double /*xoff*/, double yoff) {
    auto* cam = static_cast<Camera*>(glfwGetWindowUserPointer(w));
    if (cam) cam->scroll_delta_ += static_cast<float>(yoff);
}

void Camera::update(GLFWwindow* window, float /*delta_time*/) {
    // Zoom from scroll
    radius_ -= scroll_delta_ * 0.1f * radius_;
    radius_  = std::clamp(radius_, kRadiusMin, kRadiusMax);
    scroll_delta_ = 0.0f;

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    const bool left_held  = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)   == GLFW_PRESS;
    const bool mid_held   = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    const bool shift_held = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
                         || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    if (first_mouse_) {
        last_x_      = mx;
        last_y_      = my;
        first_mouse_ = false;
    }

    const float dx = static_cast<float>(mx - last_x_);
    const float dy = static_cast<float>(my - last_y_);
    last_x_ = mx;
    last_y_ = my;

    if (left_held && !shift_held) {
        // Orbit
        theta_ -= dx * 0.005f;
        phi_   -= dy * 0.005f;
        phi_    = std::clamp(phi_, -kPhiLimit, kPhiLimit);
    } else if (mid_held || (left_held && shift_held)) {
        // Pan focus
        const glm::vec3 forward = glm::normalize(focus_ - position());
        const glm::vec3 right   = glm::normalize(glm::cross(forward, {0.0f, 1.0f, 0.0f}));
        const glm::vec3 up      = glm::cross(right, forward);
        const float     pan_scale = radius_ * 0.001f;
        focus_ -= right * (dx * pan_scale);
        focus_ += up    * (dy * pan_scale);
    }
}

glm::vec3 Camera::position() const {
    return focus_ + glm::vec3{
        radius_ * std::cos(phi_) * std::cos(theta_),
        radius_ * std::sin(phi_),
        radius_ * std::cos(phi_) * std::sin(theta_),
    };
}

glm::mat4 Camera::view() const {
    const glm::vec3 pos = position();
    const glm::vec3 up  = {0.0f, 1.0f, 0.0f};
    return glm::lookAt(pos, focus_, up);
}

glm::mat4 Camera::projection(float aspect) const {
    return glm::perspective(glm::radians(kFov), aspect, 0.001f, 1000.0f);
}

} // namespace cs
