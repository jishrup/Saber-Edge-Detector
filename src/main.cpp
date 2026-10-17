
/**
 * @file main.cpp
 * @brief Entry point for the Saber Edge Detector application.
 *
 * Handles command-line arguments, grayscale image loading,
 * Sobel edge detection, and output image writing.
 */

#include "SobelFilter.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

/**
 * @brief Applies Sobel edge detection to an input image.
 *
 * Expects an input image path, optionally an output image path,
 * and optionally a gradient magnitude threshold.
 * Uses a default gradient magnitude threshold of 100.0.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return EXIT_SUCCESS if processing completes successfully;
 *         EXIT_FAILURE otherwise.
 */
int main(int argc, char* argv[])
{
    if (argc < 2 || argc > 4)
    {
        std::cerr << "Usage: sobel_detector <input_image> [output_image] [threshold]\n";
        return EXIT_FAILURE;
    }

    const std::string input_path = argv[1];
    const std::string output_path = argc >= 3 ? argv[2] : "sobel_output.png";

    double threshold = 100.0;

    try
    {
        if (argc == 4)
        {
            const std::string threshold_argument = argv[3];
            std::size_t parsed_length = 0;

            threshold = std::stod(threshold_argument, &parsed_length);

            if (parsed_length != threshold_argument.length() || !std::isfinite(threshold) || threshold < 0.0)
            {
                throw std::invalid_argument("Threshold must be a finite, non-negative number.");
            }
        }
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Invalid threshold: " << exception.what() << '\n';
        return EXIT_FAILURE;
    }

    // Load the input image as an 8-bit grayscale matrix.
    const cv::Mat input_image = cv::imread(input_path, cv::IMREAD_GRAYSCALE);

    if (input_image.empty())
    {
        std::cerr << "Failed to load image: " << input_path << '\n';
        return EXIT_FAILURE;
    }

    try
    {
        // Apply the naive Sobel operator without modifying the input image.
        const cv::Mat output_image = saber::SobelFilter::ApplyNaive(input_image, threshold);

        // Write the binary edge image to the requested output path.
        if (!cv::imwrite(output_path, output_image))
        {
            std::cerr << "Failed to save image: " << output_path << '\n';
            return EXIT_FAILURE;
        }

        std::cout << "Sobel edge detection completed successfully\n";
        std::cout << "Input: " << input_path << '\n';
        std::cout << "Output: " << output_path << '\n';
        std::cout << "Threshold: " << threshold << '\n';
        std::cout << "Dimensions: " << output_image.cols << " x " << output_image.rows << '\n';
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Image processing failed: " << exception.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}