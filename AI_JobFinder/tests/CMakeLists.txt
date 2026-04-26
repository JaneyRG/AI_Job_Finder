cmake_minimum_required(VERSION 3.14)
project(dblib_tests)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Add test executable
add_executable(
  dblib_tests
  SqlDbHandlerTests.cpp
  pch.cpp
)

# Link test executable against gtest and dblib
target_link_libraries(
  dblib_tests
  GTest::gtest_main
  dblib  # Your dblib library
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(dblib_tests)