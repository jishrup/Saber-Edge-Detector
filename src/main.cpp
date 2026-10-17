/**
 * @file main.cpp
 * @brief Entry point for the Saber Edge Detector application.
 *
 * Handles command-line arguments, loads grayscale images,
 * and validates image properties.
 */

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

/**
 * @brief Loads a grayscale image and displays its properties.
 *
 * Expects a single command-line argument containing the path
 * to a PNG or JPEG image.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return EXIT_SUCCESS if the image loads successfully;
 *         EXIT_FAILURE otherwise.
 */
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: sobel_detector <input_image>\n";
        return EXIT_FAILURE;
    }

    const std::string input_path = argv[1];

    // Decode the input image into an 8-bit grayscale matrix.
    const cv::Mat input_image = cv::imread(input_path, cv::IMREAD_GRAYSCALE);

    if (input_image.empty())
    {
        std::cerr << "Failed to load image: " << input_path << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "Image loaded successfully\n";
    std::cout << "Width: " << input_image.cols << '\n';
    std::cout << "Height: " << input_image.rows << '\n';
    std::cout << "Channels: " << input_image.channels() << '\n';

    return EXIT_SUCCESS;
}