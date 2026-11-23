cmake_minimum_required(VERSION 3.16)
project(ng_plus_plus_rac1_core LANGUAGES CXX)

# The progression core is kept independent of game memory and OpenRAC hooks so
# its save format and level math can be tested before kill attribution is wired.
add_library(ng-plus-plus-rac1-core STATIC
    src/progression.cpp
)
target_compile_features(ng-plus-plus-rac1-core PUBLIC cxx_std_17)
target_include_directories(ng-plus-plus-rac1-core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
set_target_properties(ng-plus-plus-rac1-core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
