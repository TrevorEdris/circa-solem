#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "circa-solem/shader_program.hpp"

#include <cstdio>
#include <cstdlib>

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

    cs::ShaderProgram shader;
    if (!shader.load(SHADERS_DIR "passthrough.vert", SHADERS_DIR "passthrough.frag")) {
        fprintf(stderr, "Failed to load shaders\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClearColor(0.05f, 0.05f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
