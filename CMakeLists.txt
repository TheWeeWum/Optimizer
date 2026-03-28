cmake_minimum_required(VERSION 3.14)

project(Optimizer
    VERSION 1.0.0
    DESCRIPTION "A C++ runtime profiling library compatible with Valgrind/KCachegrind"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Library target
# ---------------------------------------------------------------------------
add_library(optimizer STATIC
    src/optimizer.cpp
)

target_include_directories(optimizer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_options(optimizer PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
option(OPTIMIZER_BUILD_TESTS "Build the Optimizer test suite" ON)

if(OPTIMIZER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------
include(GNUInstallDirs)

install(TARGETS optimizer
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
