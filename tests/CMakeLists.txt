cmake_minimum_required(VERSION 3.14)

# ---------------------------------------------------------------------------
# Fetch GoogleTest
# ---------------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

# Prevent GoogleTest from overriding our compiler/linker settings.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ---------------------------------------------------------------------------
# Test executable
# ---------------------------------------------------------------------------
add_executable(optimizer_tests
    test_optimizer.cpp
)

target_link_libraries(optimizer_tests
    PRIVATE
        optimizer
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(optimizer_tests)
