cmake_minimum_required(VERSION 3.16)

project(SaberEdgeDetector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED COMPONENTS core imgcodecs)

add_executable(sobel_detector src/main.cpp)

target_include_directories(sobel_detector PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(sobel_detector PRIVATE ${OpenCV_LIBS})