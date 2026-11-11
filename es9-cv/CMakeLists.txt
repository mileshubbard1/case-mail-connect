cmake_minimum_required(VERSION 3.22)

set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
project(ES9CV VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The plugin needs JUCE (fetched automatically). The core + tests build anywhere.
option(ES9CV_BUILD_PLUGIN "Build the JUCE Audio Unit plugin" ON)

add_library(es9cv_core STATIC core/src/Engine.cpp)
target_include_directories(es9cv_core PUBLIC core/include)
set_target_properties(es9cv_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

enable_testing()
add_executable(es9cv_tests tests/test_engine.cpp)
target_link_libraries(es9cv_tests PRIVATE es9cv_core)
add_test(NAME es9cv_tests COMMAND es9cv_tests)

if(ES9CV_BUILD_PLUGIN)
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
    add_subdirectory(plugin)
endif()
