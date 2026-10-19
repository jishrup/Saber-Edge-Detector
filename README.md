# Saber Edge Detector

A C++17 implementation of the Sobel edge detection algorithm, with naive, optimized, and multithreaded versions.

The project uses OpenCV for image I/O, GoogleTest for unit testing, Google Benchmark for performance measurements, and Docker for distribution.

## Build and Run

### Using Docker

Build the Docker image:

```bash
./build.sh
```

Process an image:

```bash
./run.sh samples/fruits.jpg
```

Or specify an output directory:

```bash
./run.sh samples/fruits.jpg output
```

The output file is named `sobel_output.png`. By default, it is saved in the same directory as the input image.

### Local Build

Requires a C++17 compiler, CMake, and OpenCV.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run the detector:

```bash
./build/sobel_detector samples/fruits.jpg output.png
```

An optional threshold can be provided as the third argument. The default is 100.

## Design Choices

- Data structures: `cv::Mat` stores the input and output images. The implementation works with 8-bit grayscale images.
- Memory management: The input is passed by constant reference to avoid unnecessary copies. A separate output image is allocated, and OpenCV manages its memory.
- Edge handling: Sobel requires a 3x3 neighborhood. Only interior pixels are processed, and border pixels remain zero. Images smaller than 3x3 produce an all-black output.
- Edge detection: Horizontal and vertical Sobel gradients are calculated for each interior pixel. The gradient magnitude is compared against a threshold to produce a binary image.

## Performance Considerations

The naive implementation uses `cv::Mat::at()` for pixel access.

The optimized implementation uses row pointers and expanded Sobel calculations to reduce indexing overhead.

The multithreaded implementation divides image rows between worker threads. Each thread reads the shared input and writes to a separate output region. Small images use sequential processing to avoid unnecessary threading overhead.

### Benchmark Results

Mean wall-clock times measured on Apple Silicon using Google Benchmark:

| Resolution | Naive | Optimized | Parallel (4 threads) | OpenCV |
|---|---:|---:|---:|---:|
| 640x480 | 0.800 ms | 0.508 ms | 0.209 ms | 0.453 ms |
| 1280x720 | 2.470 ms | 1.580 ms | 0.523 ms | 1.300 ms |
| 1920x1080 | 5.390 ms | 3.430 ms | 1.090 ms | 2.610 ms |

At 1920x1080, the four-thread implementation was approximately 3.15x faster than the optimized single-threaded version.

Results may vary depending on hardware and system load.

### Complexity

For an image containing N pixels:

- Time complexity: O(N), since each pixel requires a constant amount of computation.
- Output memory: O(N).
- Additional threading overhead: O(T), where T is the number of worker threads.

## Testing and Benchmarks

Build and run the unit tests:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The project includes 31 tests covering edge detection, invalid inputs, threshold validation, and consistency between implementations.

To run performance benchmarks:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DSABER_BUILD_BENCHMARKS=ON
cmake --build build-release
./build-release/benchmarks/sobel_benchmark
```

## Docker Optimization Strategy

The Dockerfile uses a multi-stage build with Ubuntu 24.04.

- The build stage installs the compiler, CMake, and OpenCV development dependencies.
- The runtime stage contains the executable and required OpenCV runtime libraries, without the compiler or development tools.
- `--no-install-recommends` and package-cache cleanup reduce unnecessary image contents.
- `.dockerignore` excludes build artifacts and unrelated files from the build context.
- Docker layer caching allows unchanged build steps to be reused.

## Limitations and Improvements

- The CLI currently runs the naive implementation. A command-line option could allow users to select optimized or multithreaded processing.
- A reusable thread pool could reduce overhead when processing multiple images.
- Support could be extended to additional image formats and color images.
- SIMD optimization and further profiling could improve performance.

## Peer Review

Feedback on the supplied `ImageProcessor.h` and `ImageProcessor.cpp` is available in [PEER_REVIEW.md](PEER_REVIEW.md).