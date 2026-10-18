
# Fetch Google Benchmark for performance measurements.
include(FetchContent)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    googlebenchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.9.4.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(googlebenchmark)

# Build the Sobel benchmark executable.
add_executable(sobel_benchmark
    benchmark_sobel.cpp
)

target_link_libraries(sobel_benchmark PRIVATE
    sobel_filter
    benchmark::benchmark
    ${OpenCV_LIBS}
)
