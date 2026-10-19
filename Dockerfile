# Stage 1: Build the Sobel detector.
FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libopencv-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY CMakeLists.txt ./
COPY include/ include/
COPY src/ src/

RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF \
    -DSABER_BUILD_BENCHMARKS=OFF \
    && cmake --build build --parallel 4

# Stage 2: Run the detector without build tools.
FROM ubuntu:24.04 AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
    libopencv-core406t64 \
    libopencv-imgcodecs406t64 \
    libopencv-imgproc406t64 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/sobel_detector /usr/local/bin/sobel_detector

ENTRYPOINT ["sobel_detector"]
