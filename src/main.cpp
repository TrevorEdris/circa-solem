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
#include "circa-solem/texture.hpp"
#include "circa-solem/ring_mesh.hpp"
#include "circa-solem/particle_field.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
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
    cs::SimLoop*      sim_loop       = nullptr;
    cs::ScaleConfig*  scale          = nullptr;
    cs::Camera*       camera         = nullptr;
    cs::BodyRegistry* registry       = nullptr;
    int               warp_idx       = 6;   // default 50,000×
    int               prev_warp_idx  = 6;
    bool              trails_visible = true;
    int               focus_body_idx = -1;  // -1 = origin (no body focus)
    GLFWwindow*       window         = nullptr;
    bool              click_pending  = false;
    double            click_x        = 0.0;
    double            click_y        = 0.0;
};

static AppState* g_state = nullptr;

// Compute a good zoom radius for focusing on a body.
// Bodies with moons get a wider view; others get ~10× their display radius.
static float zoomRadiusForBody(const cs::Body& b, float size_scale) {
    const float radius_au = static_cast<float>(b.radius_km) / static_cast<float>(cs::kKmPerAU);
    const float display_r = radius_au * size_scale;
    // For gas giants / planets with known moon systems, use a wider radius
    if (b.name == "Jupiter") return 0.05f;   // shows Galilean moons (~0.013 AU max)
    if (b.name == "Saturn")  return 0.02f;   // shows Titan (~0.008 AU)
    if (b.name == "Uranus")  return 0.005f;  // shows Titania/Oberon (~0.004 AU)
    if (b.name == "Neptune") return 0.005f;  // shows Triton (~0.002 AU)
    if (b.name == "Pluto")   return 0.001f;  // shows Charon
    if (b.name == "Earth")   return 0.01f;   // shows Moon (~0.0026 AU)
    // Default: 10× display radius, clamped to at least 0.02 AU
    return std::max(display_r * 10.0f, 0.02f);
}

static void focus_on_body(int body_idx) {
    if (!g_state || !g_state->camera || !g_state->registry) return;
    const auto& bodies = g_state->registry->bodies();
    if (body_idx < 0 || body_idx >= static_cast<int>(bodies.size())) return;

    const auto& b = bodies[body_idx];
    const glm::vec3 pos = glm::vec3(b.position) * g_state->scale->distance_scale;
    g_state->camera->setFocus(pos);
    g_state->camera->setRadius(zoomRadiusForBody(b, g_state->scale->size_scale));
    g_state->focus_body_idx = body_idx;
}

static void mouse_button_callback(GLFWwindow* w, int button, int action, int mods) {
    if (!g_state) return;
    if (button != GLFW_MOUSE_BUTTON_LEFT || mods) return;

    if (action == GLFW_PRESS) {
        // Record press position to detect clicks vs. drags
        glfwGetCursorPos(w, &g_state->click_x, &g_state->click_y);
    } else if (action == GLFW_RELEASE) {
        // Only pick if mouse barely moved (< 5 pixels) — otherwise it was a drag
        double rx, ry;
        glfwGetCursorPos(w, &rx, &ry);
        const double dx = rx - g_state->click_x;
        const double dy = ry - g_state->click_y;
        if (dx * dx + dy * dy < 25.0) {
            g_state->click_x = rx;
            g_state->click_y = ry;
            g_state->click_pending = true;
        }
    }
}

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
    case GLFW_KEY_RIGHT_BRACKET: {  // ']' — focus next body
        if (!g_state->registry) break;
        const int n = static_cast<int>(g_state->registry->bodies().size());
        int next = g_state->focus_body_idx + 1;
        if (next >= n) next = 0;
        focus_on_body(next);
        break;
    }
    case GLFW_KEY_LEFT_BRACKET: {   // '[' — focus previous body
        if (!g_state->registry) break;
        const int n = static_cast<int>(g_state->registry->bodies().size());
        int prev = g_state->focus_body_idx - 1;
        if (prev < 0) prev = n - 1;
        focus_on_body(prev);
        break;
    }
    case GLFW_KEY_HOME: {           // Home — reset to origin view
        if (!g_state->camera) break;
        g_state->camera->setFocus({0.0f, 0.0f, 0.0f});
        g_state->camera->setRadius(3.0f);
        g_state->focus_body_idx = -1;
        break;
    }
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

    cs::EphemerisProvider ephemeris(EPHEMERIS_DIR);

    // ── Scene setup ───────────────────────────────────────────────────────────

    cs::BodyRegistry registry;
    cs::SimClock     clock;
    const double     jd_start = clock.julianDate();

    // Data-driven body registration: {data, naif_id, parent_name}
    struct BodyInit {
        const cs::data::BodyData& data;
        int   naif_id;
        const char* parent;  // nullptr = orbits Sun
    };

    const BodyInit body_inits[] = {
        // Sun (sentinel ID 0 → placed at origin)
        {cs::data::SUN,      0,                                nullptr},
        // Inner planets
        {cs::data::MERCURY,  cs::EphemerisProvider::MERCURY,   nullptr},
        {cs::data::VENUS,    cs::EphemerisProvider::VENUS,     nullptr},
        {cs::data::EARTH,    cs::EphemerisProvider::EARTH,     nullptr},
        {cs::data::MARS,     cs::EphemerisProvider::MARS,      nullptr},
        {cs::data::LUNA,     cs::EphemerisProvider::MOON,      "Earth"},
        // Outer planets
        {cs::data::JUPITER,  cs::EphemerisProvider::JUPITER,   nullptr},
        {cs::data::SATURN,   cs::EphemerisProvider::SATURN,    nullptr},
        {cs::data::URANUS,   cs::EphemerisProvider::URANUS,    nullptr},
        {cs::data::NEPTUNE,  cs::EphemerisProvider::NEPTUNE,   nullptr},
        // Dwarf planets
        {cs::data::PLUTO,    cs::EphemerisProvider::PLUTO,     nullptr},
        {cs::data::CERES,    cs::EphemerisProvider::CERES,     nullptr},
        // Galilean moons
        {cs::data::IO,       cs::EphemerisProvider::IO,        "Jupiter"},
        {cs::data::EUROPA,   cs::EphemerisProvider::EUROPA,    "Jupiter"},
        {cs::data::GANYMEDE, cs::EphemerisProvider::GANYMEDE,  "Jupiter"},
        {cs::data::CALLISTO, cs::EphemerisProvider::CALLISTO,  "Jupiter"},
        // Saturnian moons
        {cs::data::TITAN,    cs::EphemerisProvider::TITAN,     "Saturn"},
        {cs::data::RHEA,     cs::EphemerisProvider::RHEA,      "Saturn"},
        // Uranian moons
        {cs::data::TITANIA,  cs::EphemerisProvider::TITANIA,   "Uranus"},
        {cs::data::OBERON,   cs::EphemerisProvider::OBERON,    "Uranus"},
        // Neptunian moons
        {cs::data::TRITON,   cs::EphemerisProvider::TRITON,    "Neptune"},
        // Plutonian moons
        {cs::data::CHARON,   cs::EphemerisProvider::CHARON,    "Pluto"},
    };

    for (const auto& init : body_inits) {
        cs::Body b;
        b.name      = init.data.name;
        b.mass      = init.data.mass_msun;
        b.radius_km = init.data.radius_km;
        b.color     = {init.data.r, init.data.g, init.data.b};
        b.type      = cs::BodyType::SIMULATED;
        b.parent    = init.parent ? init.parent : "";
        if (init.naif_id == 0) {
            b.position = {0.0, 0.0, 0.0};
            b.velocity = {0.0, 0.0, 0.0};
        } else {
            const auto sv = ephemeris.getStateVector(init.naif_id, jd_start);
            b.position = sv.position_au;
            b.velocity = sv.velocity_au_yr;
        }
        registry.add(std::move(b));
    }

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
    state.camera   = &camera;
    state.registry = &registry;
    state.window   = window;
    g_state        = &state;
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
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
        // Orbit rings for bodies orbiting the Sun only (skip Sun itself and all moons).
        // Moon orbit rings are sub-pixel at solar system scale; deferred to Phase 5.
        if (b.name != "Sun" && b.parent.empty()) {
            const double r_mag = glm::length(b.position);
            const double v_sq  = glm::dot(b.velocity, b.velocity);

            // Semi-major axis (vis-viva)
            const double a = kMu * r_mag / (2.0 * kMu - r_mag * v_sq);

            // Eccentricity vector: points from focus toward periapsis
            const double rdotv = glm::dot(b.position, b.velocity);
            const glm::dvec3 e_vec =
                ((v_sq - kMu / r_mag) * b.position - rdotv * b.velocity) / kMu;
            const double e = glm::length(e_vec);

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

            // Argument of periapsis ω: angle of the eccentricity vector within
            // the orbital plane. Transform e_vec back to the XZ frame (undo the
            // plane-tilt rotation) and take atan2 in XZ.
            float omega = 0.0f;
            if (e > 1e-6) {
                const glm::mat4 inv_rot = glm::transpose(rot);
                const glm::vec3 e_local = glm::vec3(
                    inv_rot * glm::vec4(glm::normalize(glm::vec3(e_vec)), 0.0f));
                omega = std::atan2(e_local.z, e_local.x);
            }

            orbit_rings.emplace_back(
                static_cast<float>(a), static_cast<float>(e), omega);
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

    // ── Planet ring systems ─────────────────────────────────────────────────

    // Saturn ring texture: try file first, fall back to procedural
    GLuint saturn_ring_tex = cs::loadTexture(TEXTURES_DIR "saturn_ring.png");
    if (!saturn_ring_tex) saturn_ring_tex = cs::generateSaturnRingTexture();

    GLuint faint_ring_tex = cs::generateFaintRingTexture();

    // Ring dimensions in AU
    const float saturn_inner  = static_cast<float>(cs::kSaturnRingInnerKm  / cs::kKmPerAU);
    const float saturn_outer  = static_cast<float>(cs::kSaturnRingOuterKm  / cs::kKmPerAU);
    const float uranus_inner  = static_cast<float>(cs::kUranusRingInnerKm  / cs::kKmPerAU);
    const float uranus_outer  = static_cast<float>(cs::kUranusRingOuterKm  / cs::kKmPerAU);
    const float neptune_inner = static_cast<float>(cs::kNeptuneRingInnerKm / cs::kKmPerAU);
    const float neptune_outer = static_cast<float>(cs::kNeptuneRingOuterKm / cs::kKmPerAU);

    cs::RingMesh saturn_ring(saturn_inner, saturn_outer);
    cs::RingMesh uranus_ring(uranus_inner, uranus_outer);
    cs::RingMesh neptune_ring(neptune_inner, neptune_outer);

    // Axial tilt rotations: rotate ring plane from ecliptic (XZ) by planet obliquity.
    // Simplified: rotate around X axis by the obliquity angle.
    const glm::mat4 saturn_tilt  = glm::rotate(glm::mat4{1.0f}, cs::kSaturnObliquity,  {1.0f, 0.0f, 0.0f});
    const glm::mat4 uranus_tilt  = glm::rotate(glm::mat4{1.0f}, cs::kUranusObliquity,  {1.0f, 0.0f, 0.0f});
    const glm::mat4 neptune_tilt = glm::rotate(glm::mat4{1.0f}, cs::kNeptuneObliquity, {1.0f, 0.0f, 0.0f});

    // Cache body indices for Saturn, Uranus, Neptune (used each frame for ring position)
    int saturn_idx = -1, uranus_idx = -1, neptune_idx = -1;
    {
        const auto& bodies = registry.bodies();
        for (int i = 0; i < static_cast<int>(bodies.size()); ++i) {
            if (bodies[i].name == "Saturn")  saturn_idx  = i;
            if (bodies[i].name == "Uranus")  uranus_idx  = i;
            if (bodies[i].name == "Neptune") neptune_idx = i;
        }
    }

    // ── Asteroid & Kuiper belts ──────────────────────────────────────────────

    cs::ParticleField asteroid_belt(
        5000, 2.1f, 3.3f, 0.15f, 123,
        {{2.06f, 0.02f}, {2.50f, 0.03f}, {2.82f, 0.02f}, {2.95f, 0.015f}, {3.27f, 0.03f}});

    cs::ParticleField kuiper_belt(
        2500, 30.0f, 55.0f, 0.17f, 456);

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
    cs::ShaderProgram ring_shader;
    if (!ring_shader.load(SHADERS_DIR "ring.vert", SHADERS_DIR "ring.frag")) {
        fprintf(stderr, "Failed to load ring shaders\n");
        glfwDestroyWindow(window);  glfwTerminate();  return EXIT_FAILURE;
    }
    cs::ShaderProgram particle_shader;
    if (!particle_shader.load(SHADERS_DIR "particle.vert", SHADERS_DIR "particle.frag")) {
        fprintf(stderr, "Failed to load particle shaders\n");
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

        // If focused on a body, track its position each frame.
        // Release tracking if the user pans (shift+left-drag or middle-drag).
        if (state.focus_body_idx >= 0) {
            const bool left_held  = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)   == GLFW_PRESS;
            const bool mid_held   = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
            const bool shift_held = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
                                 || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
            if (mid_held || (left_held && shift_held)) {
                state.focus_body_idx = -1;
            } else {
                const auto& fb = registry.bodies()[state.focus_body_idx];
                camera.setFocus(glm::vec3(fb.position) * scale.distance_scale);
            }
        }

        // Cap effective warp speed when zoomed in so moon motion stays smooth.
        // At 0.05 AU (Jupiter moons): max warp ≈ 1000×.
        // At 3 AU (solar system): no cap (user-selected warp applies).
        const double user_warp = kWarpLevels[state.warp_idx];
        const double max_warp  = static_cast<double>(camera.radius()) * 20000.0;
        const double eff_warp  = std::min(user_warp, max_warp);
        sim_loop.setWarpFactor(eff_warp);

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
        // light_dir is computed per-body in the planet loop below.

        // Moons are only visible when the camera is close enough to see them
        // outside their parent's display sphere (< 1 AU zoom).
        const bool show_moons = camera.radius() < 1.0f;

        // When zoomed in close, reduce size_scale so bodies don't engulf the camera.
        // At 0.05 AU (Jupiter moon view): effective scale ≈ 10×.
        // At 3 AU (solar system view): effective scale = 1000× (unchanged).
        const float effective_size_scale =
            std::min(scale.size_scale, camera.radius() * 200.0f);

        // ── Click-to-pick body ───────────────────────────────────────────────
        if (state.click_pending) {
            state.click_pending = false;

            // Convert screen coords to NDC
            int win_w, win_h;
            glfwGetWindowSize(window, &win_w, &win_h);
            const float ndc_x = static_cast<float>(2.0 * state.click_x / win_w - 1.0);
            const float ndc_y = static_cast<float>(1.0 - 2.0 * state.click_y / win_h);

            // Unproject to world-space ray
            const glm::mat4 inv_vp = glm::inverse(proj * view);
            const glm::vec4 near4 = inv_vp * glm::vec4(ndc_x, ndc_y, -1.0f, 1.0f);
            const glm::vec4 far4  = inv_vp * glm::vec4(ndc_x, ndc_y,  1.0f, 1.0f);
            const glm::vec3 near_pt = glm::vec3(near4) / near4.w;
            const glm::vec3 far_pt  = glm::vec3(far4)  / far4.w;
            const glm::vec3 ray_dir = glm::normalize(far_pt - near_pt);
            const glm::vec3 ray_origin = near_pt;

            // Test ray-sphere intersection for each body, pick closest hit
            int    best_idx  = -1;
            float  best_dist = 1e30f;
            for (int i = 0; i < static_cast<int>(registry.bodies().size()); ++i) {
                const auto& b = registry.bodies()[i];
                if (!show_moons && !b.parent.empty()) continue;

                const glm::vec3 center = glm::vec3(b.position) * scale.distance_scale;
                const float radius_au = static_cast<float>(b.radius_km) / static_cast<float>(cs::kKmPerAU);
                // Use display radius for picking (what the user sees)
                float pick_r = (b.name == "Sun")
                    ? radius_au * std::min(effective_size_scale, 30.0f)
                    : radius_au * effective_size_scale;
                // Minimum pick radius so small/distant bodies are clickable
                pick_r = std::max(pick_r, 0.005f);

                const glm::vec3 oc = ray_origin - center;
                const float a_coeff = glm::dot(ray_dir, ray_dir);
                const float b_coeff = 2.0f * glm::dot(oc, ray_dir);
                const float c_coeff = glm::dot(oc, oc) - pick_r * pick_r;
                const float disc = b_coeff * b_coeff - 4.0f * a_coeff * c_coeff;
                if (disc >= 0.0f) {
                    const float t = (-b_coeff - std::sqrt(disc)) / (2.0f * a_coeff);
                    if (t > 0.0f && t < best_dist) {
                        best_dist = t;
                        best_idx  = i;
                    }
                }
            }

            if (best_idx >= 0) {
                focus_on_body(best_idx);
            }
        }

        // ── Starfield ─────────────────────────────────────────────────────────
        starfield.draw(view, proj, star_shader);

        // ── Asteroid & Kuiper belts (faint background particles) ─────────────
        asteroid_belt.draw(view, proj, particle_shader, {0.55f, 0.50f, 0.45f});
        kuiper_belt.draw(view, proj, particle_shader, {0.45f, 0.50f, 0.55f});

        // ── Orbit rings ───────────────────────────────────────────────────────
        for (std::size_t i = 0; i < orbit_rings.size(); ++i) {
            orbit_rings[i].draw(view, proj, flat_shader, ring_transforms[i]);
        }

        // ── Orbit trails ──────────────────────────────────────────────────────
        if (state.trails_visible) {
            for (const auto& b : registry.bodies()) {
                if (!show_moons && !b.parent.empty()) continue;
                auto it = trails.find(b.name);
                if (it != trails.end()) {
                    it->second.draw(view, proj, trail_shader, b.color);
                }
            }
        }

        // ── Sun glow (additive, before depth-sensitive geometry) ──────────────
        // The Sun's rendered radius is capped at 30× true scale regardless of
        // size_scale — applying the full 1000× display scale (4.65 AU) would put
        // the camera inside the sphere and produce the wrong visual result.
        const float sun_true_radius =
            static_cast<float>(cs::data::SUN.radius_km) / static_cast<float>(cs::kKmPerAU);
        const float sun_display_radius = sun_true_radius * std::min(effective_size_scale, 30.0f);
        sun_glow.draw(view, proj, sun_display_pos, sun_display_radius, billboard_shader);

        // ── Planets (Phong shading) ───────────────────────────────────────────
        phong_shader.use();
        set_vec3(phong_shader.id(), "light_color", light_color);
        set_mat4(phong_shader.id(), "view",        view);
        set_mat4(phong_shader.id(), "projection",  proj);
        set_vec3(phong_shader.id(), "view_pos",    camera.position());

        glEnable(GL_DEPTH_TEST);
        for (const auto& b : registry.bodies()) {
            if (!show_moons && !b.parent.empty()) continue;

            const float radius_au = static_cast<float>(b.radius_km) / static_cast<float>(cs::kKmPerAU);
            // Sun is capped at 30× true scale so the camera remains outside the sphere.
            const float display_r = (b.name == "Sun")
                ? radius_au * std::min(effective_size_scale, 30.0f)
                : radius_au * effective_size_scale;
            const glm::vec3 display_p = glm::vec3(b.position) * scale.distance_scale;

            // Per-body light direction: from body toward the Sun.
            // Sun itself is self-luminous — use a dummy direction so it's fully lit.
            const glm::vec3 body_light_dir = (b.name == "Sun")
                ? glm::normalize(camera.position() - display_p)
                : glm::normalize(sun_display_pos - display_p);
            set_vec3(phong_shader.id(), "light_dir", body_light_dir);

            glm::mat4 model = glm::translate(glm::mat4{1.0f}, display_p);
            model           = glm::scale(model, glm::vec3(display_r));
            const glm::mat3 normal_mat = glm::mat3(glm::transpose(glm::inverse(model)));

            set_mat4(phong_shader.id(), "model",        model);
            set_vec3(phong_shader.id(), "object_color", b.color);
            glUniformMatrix3fv(glGetUniformLocation(phong_shader.id(), "normal_matrix"),
                               1, GL_FALSE, glm::value_ptr(normal_mat));
            sphere.draw();
        }

        // ── Planet ring systems (alpha-blended, after opaque geometry) ────────
        {
            auto draw_planet_ring = [&](int body_idx, cs::RingMesh& mesh,
                                        const glm::mat4& tilt, GLuint tex,
                                        const glm::vec3& tint, float ring_scale) {
                if (body_idx < 0) return;
                const auto& body = registry.bodies()[body_idx];
                const glm::vec3 pos = glm::vec3(body.position) * scale.distance_scale;
                glm::mat4 model = glm::translate(glm::mat4{1.0f}, pos);
                model = model * tilt;
                model = glm::scale(model, glm::vec3(ring_scale));
                mesh.draw(view, proj, ring_shader, model, tex, tint);
            };

            // Scale rings with effective_size_scale so they match planet size
            draw_planet_ring(saturn_idx,  saturn_ring,  saturn_tilt,
                             saturn_ring_tex, {0.90f, 0.85f, 0.70f}, effective_size_scale);
            draw_planet_ring(uranus_idx,  uranus_ring,  uranus_tilt,
                             faint_ring_tex, {0.70f, 0.80f, 0.85f}, effective_size_scale);
            draw_planet_ring(neptune_idx, neptune_ring, neptune_tilt,
                             faint_ring_tex, {0.55f, 0.60f, 0.75f}, effective_size_scale);
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
