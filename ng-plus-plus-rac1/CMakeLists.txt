cmake_minimum_required(VERSION 3.20)
project(ng_plus_plus_rac1 LANGUAGES C CXX)

# Set OPENRAC_SDK to an unpacked OpenRAC mod SDK (or point OpenRACMod_DIR at
# its cmake/ directory). The resulting package contains the native plugin.
if(NOT OpenRACMod_DIR AND DEFINED OPENRAC_SDK)
    set(OpenRACMod_DIR "${OPENRAC_SDK}/cmake")
endif()
find_package(OpenRACMod 1.16 REQUIRED)

add_library(ngpp_rac1_progression STATIC src/progression.cpp)
target_compile_features(ngpp_rac1_progression PUBLIC cxx_std_17)
target_include_directories(ngpp_rac1_progression PUBLIC src)

openrac_add_mod(ng_plus_plus_rac1
    MANIFEST openrac-mod.json
    SOURCES
        src/mod.cpp
        src/progression.cpp
)

include(CTest)
if(BUILD_TESTING)
    add_executable(ng_plus_plus_rac1_progression_test tests/progression_test.cpp)
    target_link_libraries(ng_plus_plus_rac1_progression_test PRIVATE ngpp_rac1_progression)
    add_test(NAME rac1_progression COMMAND ng_plus_plus_rac1_progression_test)
endif()
