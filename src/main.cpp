#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "circa-solem/body.hpp"
#include "circa-solem/body_registry.hpp"
#include "circa-solem/camera.hpp"
#include "circa-solem/integrator.hpp"
#include "circa-solem/shader_program.hpp"
#include "circa-solem/sim_clock.hpp"
#include "circa-solem/sim_loop.hpp"
#include "circa-solem/sphere.hpp"
#include "circa-solem/starfield.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>
#include <cstdlib>
#include <cmath>

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

// ── Body definitions ──────────────────────────────────────────────────────────

static cs::Body make_sun() {
    cs::Body b;
    b.name      = "Sun";
    b.mass      = 1.0;
    b.radius_km = 696000.0;
    b.position  = {0.0, 0.0, 0.0};
    b.velocity  = {0.0, 0.0, 0.0};
    b.color     = {1.0f, 0.95f, 0.4f};
    b.type      = cs::BodyType::SIMULATED;
    return b;
}

static cs::Body make_earth() {
    // Circular orbit at 1 AU: v = sqrt(G * M_sun / r) = 2π AU/yr
    cs::Body b;
    b.name      = "Earth";
    b.mass      = 3.003e-6;
    b.radius_km = 6371.0;
    b.position  = {1.0, 0.0, 0.0};
    b.velocity  = {0.0, 2.0 * static_cast<double>(M_PI), 0.0};
    b.color     = {0.2f, 0.5f, 1.0f};
    b.type      = cs::BodyType::SIMULATED;
    return b;
}

// ── Shader uniform helpers ────────────────────────────────────────────────────

static void set_uniform_mat4(GLuint prog, const char* name, const glm::mat4& m) {
    glUniformMatrix4fv(glGetUniformLocation(prog, name), 1, GL_FALSE, glm::value_ptr(m));
}
static void set_uniform_vec3(GLuint prog, const char* name, const glm::vec3& v) {
    glUniform3fv(glGetUniformLocation(prog, name), 1, glm::value_ptr(v));
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

    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(w, GLFW_TRUE);
    });

    // ── Scene setup ───────────────────────────────────────────────────────────

    cs::BodyRegistry registry;
    registry.add(make_sun());
    registry.add(make_earth());

    cs::SimClock clock;
    cs::SimLoop  sim_loop{registry, clock, 1000.0};

    cs::Camera camera;
    camera.attachToWindow(window);

    cs::Sphere   sphere;
    cs::Starfield starfield;

    cs::ShaderProgram phong_shader;
    if (!phong_shader.load(SHADERS_DIR "phong.vert", SHADERS_DIR "phong.frag")) {
        fprintf(stderr, "Failed to load Phong shaders\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    cs::ShaderProgram star_shader;
    if (!star_shader.load(SHADERS_DIR "starfield.vert", SHADERS_DIR "starfield.frag")) {
        fprintf(stderr, "Failed to load starfield shaders\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Required for gl_PointSize in vertex shader (Core Profile).
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
    // Alpha blending for star brightness
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Fixed directional light (approximates Sun for Phase 1 — replaced in Phase 2)
    const glm::vec3 light_dir   = glm::normalize(glm::vec3(1.0f, 1.0f, 0.5f));
    const glm::vec3 light_color = {1.0f, 1.0f, 0.95f};

    // Display-scale multipliers: bodies are tiny in AU — scale up for visibility.
    // Sun: render at 0.05 AU radius. Earth: 0.02 AU radius.
    // Phase 2 introduces a proper ScaleConfig system.
    const float sun_render_r   = 0.05f;
    const float earth_render_r = 0.02f;

    double last_time = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        const double now   = glfwGetTime();
        const float  dt    = static_cast<float>(now - last_time);
        last_time          = now;

        glfwPollEvents();
        camera.update(window, dt);
        sim_loop.tick(static_cast<double>(dt));

        int fb_w, fb_h;
        glfwGetFramebufferSize(window, &fb_w, &fb_h);
        glViewport(0, 0, fb_w, fb_h);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float     aspect = static_cast<float>(fb_w) / static_cast<float>(fb_h);
        const glm::mat4 view   = camera.view();
        const glm::mat4 proj   = camera.projection(aspect);

        // ── Starfield (depth writes off, drawn first) ─────────────────────────
        starfield.draw(view, proj, star_shader);

        // ── Planets ───────────────────────────────────────────────────────────
        phong_shader.use();
        set_uniform_vec3(phong_shader.id(), "light_dir",   light_dir);
        set_uniform_vec3(phong_shader.id(), "light_color", light_color);
        set_uniform_mat4(phong_shader.id(), "view",        view);
        set_uniform_mat4(phong_shader.id(), "projection",  proj);
        set_uniform_vec3(phong_shader.id(), "view_pos",    camera.position());

        const auto& bodies = registry.bodies();
        for (const auto& body : bodies) {
            const float render_r = (body.name == "Sun") ? sun_render_r : earth_render_r;

            glm::mat4 model = glm::translate(glm::mat4{1.0f},
                                              glm::vec3(body.position));
            model = glm::scale(model, glm::vec3(render_r));

            const glm::mat3 normal_mat = glm::mat3(glm::transpose(glm::inverse(model)));

            set_uniform_mat4(phong_shader.id(), "model",         model);
            set_uniform_vec3(phong_shader.id(), "object_color",  body.color);
            glUniformMatrix3fv(glGetUniformLocation(phong_shader.id(), "normal_matrix"),
                               1, GL_FALSE, glm::value_ptr(normal_mat));

            sphere.draw();
        }

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
