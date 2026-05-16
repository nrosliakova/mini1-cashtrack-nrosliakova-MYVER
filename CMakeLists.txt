cmake_minimum_required(VERSION 3.20)
project(cashtrack VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# nlohmann/json is vendored as a single header in vendor/
include_directories(${CMAKE_SOURCE_DIR}/vendor)

# --- Library target: everything except main() ---
add_library(cashtrack_core
    src/cli.cpp
    src/transaction.cpp
    src/importer.cpp
    src/reporter.cpp
    src/categories.cpp
    src/storage.cpp
)
target_include_directories(cashtrack_core PUBLIC include)

# --- Executable ---
add_executable(cashtrack src/main.cpp)
target_link_libraries(cashtrack PRIVATE cashtrack_core)

# --- Tests (GoogleTest via FetchContent) ---
option(CASHTRACK_BUILD_TESTS "Build cashtrack tests" ON)
if(CASHTRACK_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
        GIT_SHALLOW    TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_subdirectory(tests)
endif()

# --- Install ---
install(TARGETS cashtrack DESTINATION bin)
