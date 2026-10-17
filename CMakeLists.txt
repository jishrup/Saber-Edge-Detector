cmake_minimum_required(VERSION 3.16)

project(SaberEdgeDetector LANGUAGES CXX)

# Require C++17 without compiler-specific extensions.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Locate the OpenCV components required by the application.
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs)

# Define the executable and its source files.
add_executable(sobel_detector
    src/main.cpp
    src/SobelFilter.cpp
)

# Make project headers and OpenCV headers available to the compiler.
target_include_directories(sobel_detector PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

# Link the executable against the required OpenCV libraries.
target_link_libraries(sobel_detector PRIVATE
    ${OpenCV_LIBS}
)