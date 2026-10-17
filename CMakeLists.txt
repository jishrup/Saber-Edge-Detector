cmake_minimum_required(VERSION 3.16)

project(SaberEdgeDetector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(sobel_detector src/main.cpp)