
# Fetch GoogleTest for unit and integration testing.
include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(googletest)

# Build the Sobel test executable.
add_executable(sobel_tests
    test_sobel.cpp
    test_cli.cpp
)

target_link_libraries(sobel_tests PRIVATE
    sobel_filter
    GTest::gtest_main
)

# Provide absolute paths for CLI integration testing.
target_compile_definitions(sobel_tests PRIVATE
    SOBEL_DETECTOR_PATH="$<TARGET_FILE:sobel_detector>"
    SOBEL_SAMPLE_PATH="${PROJECT_SOURCE_DIR}/samples/test_edge.png"
)

# Build the application before running its integration tests.
add_dependencies(sobel_tests sobel_detector)

include(GoogleTest)
gtest_discover_tests(sobel_tests)
