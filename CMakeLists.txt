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

# ── Main executable ───────────────────────────────────────────────────────────
file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(circa-solem ${SOURCES})

target_include_directories(circa-solem PRIVATE include)

target_link_libraries(circa-solem PRIVATE
    OpenGL::GL
    glfw
    glm::glm
    glad
    stb
)

# Absolute path to shaders dir; works with any out-of-source build directory
target_compile_definitions(circa-solem PRIVATE
    SHADERS_DIR="${CMAKE_SOURCE_DIR}/shaders/"
)

# ── Tests ─────────────────────────────────────────────────────────────────────
enable_testing()
add_subdirectory(tests)
