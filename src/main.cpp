#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "circa-solem/axis_gizmo.hpp"
#include "circa-solem/body.hpp"
#include "circa-solem/body_registry.hpp"
#include "circa-solem/camera.hpp"
#include "circa-solem/constants.hpp"
#include "circa-solem/ephemeris_provider.hpp"
#include "circa-solem/orbit_path.hpp"
#include "circa-solem/orbit_trail.hpp"
#include "circa-solem/scale_config.hpp"
#include "circa-solem/shader_program.hpp"
#include "circa-solem/sim_clock.hpp"
#include "circa-solem/sim_loop.hpp"
#include "circa-solem/solar_system_data.hpp"
#include "circa-solem/sphere.hpp"
#include "circa-solem/starfield.hpp"
#include "circa-solem/sun_glow.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

// ── Callbacks ─────────────────────────────────────────────────────────────────

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

static void GLAPIENTRY gl_debug_callback(
    GLenum /*source*/, GLenum /*type*/, GLuint id, GLenum severity,
    GLsizei /*length*/, const GLchar* message, const void* /*userParam*/)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
    fprintf(stderr, "GL [%u] %s\n", id, message);
}

// ── Shader uniform helpers ─────────────────────────────────────────────────────

static void set_mat4(GLuint prog, const char* name, const glm::mat4& m) {
    glUniformMatrix4fv(glGetUniformLocation(prog, name), 1, GL_FALSE, glm::value_ptr(m));
}
static void set_vec3(GLuint prog, const char* name, const glm::vec3& v) {
    glUniform3fv(glGetUniformLocation(prog, name), 1, glm::value_ptr(v));
}

// ── Time warp ─────────────────────────────────────────────────────────────────

static constexpr std::array<double, 8> kWarpLevels = {
    0.0, 1.0, 10.0, 100.0, 1000.0, 10000.0, 50000.0, 100000.0
};

static void update_title(GLFWwindow* w, double warp) {
    if (warp == 0.0) {
        glfwSetWindowTitle(w, "Circa Solem — PAUSED");
    } else {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "Circa Solem — %.0f\xc3\x97", warp); // ×
        glfwSetWindowTitle(w, buf);
    }
}

// ── Per-frame state accessible from GLFW callbacks ────────────────────────────

struct AppState {
    cs::SimLoop*   sim_loop      = nullptr;
    cs::ScaleConfig* scale       = nullptr;
    int            warp_idx      = 6;   // default 50,000×
    int            prev_warp_idx = 6;
    bool           trails_visible = true;
    GLFWwindow*    window         = nullptr;
};

static AppState* g_state = nullptr;

static void key_callback(GLFWwindow* w, int key, int /*scancode*/, int action, int /*mods*/) {
    if (!g_state) return;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(w, GLFW_TRUE);
        return;
    }

    if (action != GLFW_PRESS) return;

    switch (key) {
    case GLFW_KEY_PERIOD:   // '.' — increase warp
        if (g_state->warp_idx < (int)kWarpLevels.size() - 1) {
            ++g_state->warp_idx;
            g_state->sim_loop->setWarpFactor(kWarpLevels[g_state->warp_idx]);
            update_title(w, g_state->sim_loop->warpFactor());
        }
        break;
    case GLFW_KEY_COMMA:    // ',' — decrease warp
        if (g_state->warp_idx > 0) {
            --g_state->warp_idx;
            g_state->sim_loop->setWarpFactor(kWarpLevels[g_state->warp_idx]);
            update_title(w, g_state->sim_loop->warpFactor());
        }
        break;
    case GLFW_KEY_SPACE:    // pause / resume
        if (kWarpLevels[g_state->warp_idx] != 0.0) {
            g_state->prev_warp_idx = g_state->warp_idx;
            g_state->warp_idx      = 0;
        } else {
            g_state->warp_idx      = g_state->prev_warp_idx;
        }
        g_state->sim_loop->setWarpFactor(kWarpLevels[g_state->warp_idx]);
        update_title(w, g_state->sim_loop->warpFactor());
        break;
    case GLFW_KEY_T:        // toggle scale
        if (g_state->scale->size_scale > 1.0f) {
            *g_state->scale = cs::ScaleConfig::true_scale();
        } else {
            *g_state->scale = cs::ScaleConfig::display();
        }
        break;
    case GLFW_KEY_O:        // toggle orbit trails
        g_state->trails_visible = !g_state->trails_visible;
        break;
    }
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Circa Solem", nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL(glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    if (GLAD_GL_KHR_debug) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_debug_callback, nullptr);
    }

    // ── Ephemeris ─────────────────────────────────────────────────────────────

    cs::EphemerisProvider ephemeris(EPHEMERIS_PATH);

    // ── Scene setup ───────────────────────────────────────────────────────────

    cs::BodyRegistry registry;
    cs::SimClock     clock;
    const double     jd_start = clock.julianDate();

    // Helper: build a Body from data constants + ephemeris state vector.
    auto make_body = [&](const cs::data::BodyData& d, int eph_id) {
        cs::Body b;
        b.name      = d.name;
        b.mass      = d.mass_msun;
        b.radius_km = d.radius_km;
        b.color     = {d.r, d.g, d.b};
        b.type      = cs::BodyType::SIMULATED;
        if (eph_id == 0) {
            b.position = {0.0, 0.0, 0.0};
            b.velocity = {0.0, 0.0, 0.0};
        } else {
            const auto sv = ephemeris.getStateVector(eph_id, jd_start);
            b.position = sv.position_au;
            b.velocity = sv.velocity_au_yr;
        }
        return b;
    };

    registry.add(make_body(cs::data::SUN,     0));
    registry.add(make_body(cs::data::MERCURY, cs::EphemerisProvider::MERCURY));
    registry.add(make_body(cs::data::VENUS,   cs::EphemerisProvider::VENUS));
    registry.add(make_body(cs::data::EARTH,   cs::EphemerisProvider::EARTH));
    registry.add(make_body(cs::data::MARS,    cs::EphemerisProvider::MARS));
    registry.add(make_body(cs::data::LUNA,    cs::EphemerisProvider::MOON));

    // Cache the Sun's index so we don't scan by name every frame.
    int sun_idx = 0;
    {
        const auto& bodies = registry.bodies();
        for (int i = 0; i < static_cast<int>(bodies.size()); ++i) {
            if (bodies[i].name == "Sun") { sun_idx = i; break; }
        }
    }

    cs::SimLoop sim_loop{registry, clock};  // default 50,000×
    cs::Camera  camera;
    camera.attachToWindow(window);

    // ── Scale & app state ─────────────────────────────────────────────────────

    cs::ScaleConfig scale = cs::ScaleConfig::display();

    AppState state;
    state.sim_loop = &sim_loop;
    state.scale    = &scale;
    state.window   = window;
    g_state        = &state;
    glfwSetKeyCallback(window, key_callback);
    update_title(window, sim_loop.warpFactor());

    // ── Render objects ────────────────────────────────────────────────────────

    cs::Sphere    sphere;
    cs::Starfield starfield;
    cs::AxisGizmo axis_gizmo;
    cs::SunGlow   sun_glow;

    // One static orbit ring per non-Moon body (Moon orbit tiny, skip for clarity).
    // Ring radius = semi-major axis (vis-viva), ring plane = actual orbital plane (r×v).
    // μ = GM_sun = 4π² AU³/yr² in our units.
    constexpr double kMu = 4.0 * 3.14159265358979323846 * 3.14159265358979323846;

    std::vector<cs::OrbitPath> orbit_rings;
    std::vector<glm::mat4>     ring_transforms;

    for (const auto& b : registry.bodies()) {
        if (b.name != "Sun" && b.name != "Moon") {
            const double r  = glm::length(b.position);
            const double v2 = glm::dot(b.velocity, b.velocity);
            // Vis-viva: a = μr / (2μ − rv²)
            const double a  = kMu * r / (2.0 * kMu - r * v2);
            orbit_rings.emplace_back(static_cast<float>(a));

            // Orbital plane normal = normalize(r × v). Ring is drawn in XZ plane
            // with normal Y=(0,1,0); rotate Y to h_hat to match the actual plane.
            const glm::dvec3 h     = glm::cross(b.position, b.velocity);
            const glm::dvec3 h_hat = glm::normalize(h);
            const glm::vec3  from  = {0.0f, 1.0f, 0.0f};
            const glm::vec3  to    = glm::vec3(h_hat);
            const glm::vec3  axis  = glm::cross(from, to);
            const float      sin_a = glm::length(axis);
            const float      cos_a = glm::dot(from, to);
            glm::mat4 rot{1.0f};
            if (sin_a > 1e-6f) {
                rot = glm::rotate(glm::mat4{1.0f}, std::atan2(sin_a, cos_a),
                                  glm::normalize(axis));
            }
            ring_transforms.push_back(rot);
        }
    }

    // One trail per non-Sun body
    std::map<std::string, cs::OrbitTrail> trails;
    for (const auto& b : registry.bodies()) {
        if (b.name != "Sun") {
            trails.emplace(std::piecewise_construct,
                           std::forward_as_tuple(b.name),
                           std::forward_as_tuple(500));
        }
    }

    // ── Shaders ───────────────────────────────────────────────────────────────

    cs::ShaderProgram phong_shader;
    if (!phong_shader.load(SHADERS_DIR "phong.vert", SHADERS_DIR "phong.frag")) {
        fprintf(stderr, "Failed to load Phong shaders\n");
        glfwDestroyWindow(window);  glfwTerminate();  return EXIT_FAILURE;
    }
    cs::ShaderProgram star_shader;
    if (!star_shader.load(SHADERS_DIR "starfield.vert", SHADERS_DIR "starfield.frag")) {
        fprintf(stderr, "Failed to load starfield shaders\n");
        glfwDestroyWindow(window);  glfwTerminate();  return EXIT_FAILURE;
    }
    cs::ShaderProgram flat_shader;
    if (!flat_shader.load(SHADERS_DIR "flat.vert", SHADERS_DIR "flat.frag")) {
        fprintf(stderr, "Failed to load flat shaders\n");
        glfwDestroyWindow(window);  glfwTerminate();  return EXIT_FAILURE;
    }
    cs::ShaderProgram trail_shader;
    if (!trail_shader.load(SHADERS_DIR "trail.vert", SHADERS_DIR "trail.frag")) {
        fprintf(stderr, "Failed to load trail shaders\n");
        glfwDestroyWindow(window);  glfwTerminate();  return EXIT_FAILURE;
    }
    cs::ShaderProgram billboard_shader;
    if (!billboard_shader.load(SHADERS_DIR "billboard.vert", SHADERS_DIR "billboard.frag")) {
        fprintf(stderr, "Failed to load billboard shaders\n");
        glfwDestroyWindow(window);  glfwTerminate();  return EXIT_FAILURE;
    }

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);

    // Light direction follows the Sun's current display position each frame.
    const glm::vec3 light_color = {1.0f, 1.0f, 0.95f};

    double last_time      = glfwGetTime();
    int    trail_push_acc = 0;  // push trail every 10 sim steps for performance

    while (!glfwWindowShouldClose(window)) {
        const double now = glfwGetTime();
        const float  dt  = static_cast<float>(now - last_time);
        last_time        = now;

        glfwPollEvents();
        camera.update(window, dt);

        // Advance simulation and push trail positions every 10 substeps.
        sim_loop.tick(static_cast<double>(dt));
        ++trail_push_acc;
        if (trail_push_acc >= 10) {
            trail_push_acc = 0;
            for (const auto& b : registry.bodies()) {
                auto it = trails.find(b.name);
                if (it != trails.end()) {
                    it->second.push(b.position);
                }
            }
        }

        int fb_w, fb_h;
        glfwGetFramebufferSize(window, &fb_w, &fb_h);
        glViewport(0, 0, fb_w, fb_h);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float     aspect = static_cast<float>(fb_w) / static_cast<float>(fb_h);
        const glm::mat4 view   = camera.view();
        const glm::mat4 proj   = camera.projection(aspect);

        // Sun display position for lighting and glow (cached index, no string scan).
        const glm::vec3 sun_display_pos =
            glm::vec3(registry.bodies()[sun_idx].position) * scale.distance_scale;
        const glm::vec3 light_dir = glm::normalize(camera.position() - sun_display_pos);

        // ── Starfield ─────────────────────────────────────────────────────────
        starfield.draw(view, proj, star_shader);

        // ── Orbit rings ───────────────────────────────────────────────────────
        for (std::size_t i = 0; i < orbit_rings.size(); ++i) {
            orbit_rings[i].draw(view, proj, flat_shader, ring_transforms[i]);
        }

        // ── Orbit trails ──────────────────────────────────────────────────────
        if (state.trails_visible) {
            for (const auto& b : registry.bodies()) {
                auto it = trails.find(b.name);
                if (it != trails.end()) {
                    it->second.draw(view, proj, trail_shader, b.color);
                }
            }
        }

        // ── Sun glow (additive, before depth-sensitive geometry) ──────────────
        // Pass the Sun's rendered radius so the billboard wraps the sphere.
        const float sun_display_radius =
            static_cast<float>(cs::data::SUN.radius_km) / static_cast<float>(cs::kKmPerAU)
            * scale.size_scale;
        sun_glow.draw(view, proj, sun_display_pos, sun_display_radius, billboard_shader);

        // ── Planets (Phong shading) ───────────────────────────────────────────
        phong_shader.use();
        set_vec3(phong_shader.id(), "light_dir",   light_dir);
        set_vec3(phong_shader.id(), "light_color", light_color);
        set_mat4(phong_shader.id(), "view",        view);
        set_mat4(phong_shader.id(), "projection",  proj);
        set_vec3(phong_shader.id(), "view_pos",    camera.position());

        glEnable(GL_DEPTH_TEST);
        for (const auto& b : registry.bodies()) {
            const float radius_au     = static_cast<float>(b.radius_km) / static_cast<float>(cs::kKmPerAU);
            const float display_r     = radius_au * scale.size_scale;
            const glm::vec3 display_p = glm::vec3(b.position) * scale.distance_scale;

            glm::mat4 model = glm::translate(glm::mat4{1.0f}, display_p);
            model           = glm::scale(model, glm::vec3(display_r));
            const glm::mat3 normal_mat = glm::mat3(glm::transpose(glm::inverse(model)));

            set_mat4(phong_shader.id(), "model",        model);
            set_vec3(phong_shader.id(), "object_color", b.color);
            glUniformMatrix3fv(glGetUniformLocation(phong_shader.id(), "normal_matrix"),
                               1, GL_FALSE, glm::value_ptr(normal_mat));
            sphere.draw();
        }

        // ── Axis gizmo ────────────────────────────────────────────────────────
        axis_gizmo.draw(view, proj, flat_shader);

        glfwSwapBuffers(window);
    }

    g_state = nullptr;
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
