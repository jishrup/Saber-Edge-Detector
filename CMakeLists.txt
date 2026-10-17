cmake_minimum_required(VERSION 3.16)

project(SaberEdgeDetector LANGUAGES CXX)

# Require C++17 without compiler-specific extensions.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Locate the OpenCV components required by the application.
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs)

# Compile the Sobel implementation as a reusable library.
add_library(sobel_filter
    src/SobelFilter.cpp
)

target_include_directories(sobel_filter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(sobel_filter PUBLIC
    ${OpenCV_LIBS}
)

# Build the command-line application.
add_executable(sobel_detector
    src/main.cpp
)

target_link_libraries(sobel_detector PRIVATE
    sobel_filter
)

# Enable optional unit testing.
include(CTest)

if(BUILD_TESTING)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    FetchContent_MakeAvailable(googletest)

    add_executable(sobel_tests
        tests/test_sobel.cpp
    )

    target_link_libraries(sobel_tests PRIVATE
        sobel_filter
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(sobel_tests)
endif()