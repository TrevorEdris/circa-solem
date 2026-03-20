# Test harness for circa-solem.
# Non-GL tests only — SimClock, Integrator, and EphemerisProvider run without an OpenGL context.
# GL-dependent code (Sphere, Camera, Starfield) is verified manually.

include(CTest)
include(Catch)

add_executable(test_sim_clock test_sim_clock.cpp)
target_link_libraries(test_sim_clock PRIVATE Catch2::Catch2WithMain circa-solem-core)
catch_discover_tests(test_sim_clock)

add_executable(test_integrator test_integrator.cpp)
target_link_libraries(test_integrator PRIVATE Catch2::Catch2WithMain circa-solem-core)
catch_discover_tests(test_integrator)

add_executable(test_ephemeris_provider test_ephemeris_provider.cpp)
target_link_libraries(test_ephemeris_provider PRIVATE Catch2::Catch2WithMain circa-solem-core)
target_compile_definitions(test_ephemeris_provider PRIVATE
    EPHEMERIS_DIR="${CMAKE_SOURCE_DIR}/data/ephemeris/"
)
catch_discover_tests(test_ephemeris_provider)
