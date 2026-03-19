cmake_minimum_required(VERSION 3.20)
project(circa-solem
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
find_package(OpenGL REQUIRED)

# ── GLFW ──────────────────────────────────────────────────────────────────────
find_package(glfw3 3.3 QUIET)
if(NOT glfw3_FOUND)
    message(STATUS "GLFW not found via find_package; fetching via FetchContent")
    FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
endif()

# ── GLM ───────────────────────────────────────────────────────────────────────
find_package(glm QUIET)
if(NOT glm_FOUND)
    message(STATUS "GLM not found via find_package; fetching via FetchContent")
    FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.1
    )
    FetchContent_MakeAvailable(glm)
endif()

# ── GLAD (OpenGL 4.1 Core, committed to extern/glad/) ────────────────────────
add_library(glad STATIC extern/glad/src/gl.c)
target_include_directories(glad PUBLIC extern/glad/include)

# ── stb (header-only, committed to extern/stb/) ──────────────────────────────
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE extern/stb)

# ── Core library (no OpenGL/GLFW — testable without a display context) ────────
# Non-GL source files are added here as each phase is implemented.
# Tests link against circa-solem-core; GL-dependent code stays in the executable.
add_library(circa-solem-core STATIC
    src/sim_clock.cpp
    # Phase 1 (added as implemented):
    #   src/integrator.cpp
    #   src/body_registry.cpp
    #   src/sim_loop.cpp
)
target_include_directories(circa-solem-core PUBLIC include)
target_link_libraries(circa-solem-core PUBLIC glm::glm)

# ── Main executable ───────────────────────────────────────────────────────────
# GL-dependent source files are listed explicitly here.
add_executable(circa-solem
    src/main.cpp
    src/shader_program.cpp
    # Phase 1 (added as implemented):
    #   src/sphere.cpp
    #   src/camera.cpp
    #   src/starfield.cpp
)

target_link_libraries(circa-solem PRIVATE
    circa-solem-core
    OpenGL::GL
    glfw
    glad
    stb
)

# Absolute path to shaders dir; works with any out-of-source build directory
target_compile_definitions(circa-solem PRIVATE
    SHADERS_DIR="${CMAKE_SOURCE_DIR}/shaders/"
)

# ── Catch2 ────────────────────────────────────────────────────────────────────
find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
    message(STATUS "Catch2 not found via find_package; fetching via FetchContent")
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)
endif()
list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras"
                              "${Catch2_DIR}/../../../lib/cmake/Catch2")

# ── Tests ─────────────────────────────────────────────────────────────────────
enable_testing()
add_subdirectory(tests)
