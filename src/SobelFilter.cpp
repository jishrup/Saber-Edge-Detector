/**
 * @file SobelFilter.cpp
 * @brief Implements Sobel edge detection algorithms.
 */

#include "SobelFilter.h"

#include <cmath>
#include <stdexcept>

namespace saber
{

cv::Mat SobelFilter::ApplyNaive(const cv::Mat& input_image, double threshold)
{
    if (input_image.empty())
    {
        throw std::invalid_argument("Input image cannot be empty.");
    }

    if (input_image.type() != CV_8UC1)
    {
        throw std::invalid_argument("Input image must be 8-bit grayscale.");
    }

    if (!std::isfinite(threshold) || threshold < 0.0)
    {
        throw std::invalid_argument("Threshold must be finite and non-negative.");
    }

    // Initialize the output to black, including all border pixels.
    cv::Mat output_image = cv::Mat::zeros(input_image.size(), CV_8UC1);

    // A 3x3 kernel cannot be applied to images smaller than 3x3.
    if (input_image.rows < 3 || input_image.cols < 3)
    {
        return output_image;
    }

    // Process only interior pixels to avoid out-of-bounds access.
    for (int row = 1; row < input_image.rows - 1; ++row)
    {
        for (int col = 1; col < input_image.cols - 1; ++col)
        {
            int gradient_x = 0;
            int gradient_y = 0;

            // Apply both Sobel kernels to the 3x3 neighborhood.
            for (int kernel_row = -1; kernel_row <= 1; ++kernel_row)
            {
                for (int kernel_col = -1; kernel_col <= 1; ++kernel_col)
                {
                    const int pixel_value = input_image.at<unsigned char>(row + kernel_row, col + kernel_col);

                    const int kernel_y = kernel_row + 1;
                    const int kernel_x = kernel_col + 1;

                    gradient_x += pixel_value * SobelX[kernel_y][kernel_x];
                    gradient_y += pixel_value * SobelY[kernel_y][kernel_x];
                }
            }

            // Calculate gradient magnitude using the Euclidean norm.
            const double magnitude = std::sqrt(static_cast<double>(gradient_x) * gradient_x +
                                               static_cast<double>(gradient_y) * gradient_y);

            // Convert the gradient magnitude into a binary edge value.
            output_image.at<unsigned char>(row, col) = magnitude >= threshold ? 255 : 0;
        }
    }

    return output_image;
}


cv::Mat SobelFilter::ApplyOptimized(const cv::Mat& input_image, double threshold)
{
    if (input_image.empty())
    {
        throw std::invalid_argument("Input image cannot be empty.");
    }

    if (input_image.type() != CV_8UC1)
    {
        throw std::invalid_argument("Input image must be 8-bit grayscale.");
    }

    if (!std::isfinite(threshold) || threshold < 0.0)
    {
        throw std::invalid_argument("Threshold must be finite and non-negative.");
    }

    // Initialize the output to black, including all border pixels.
    cv::Mat output_image = cv::Mat::zeros(input_image.size(), CV_8UC1);

    // A 3x3 kernel cannot be applied to images smaller than 3x3.
    if (input_image.rows < 3 || input_image.cols < 3)
    {
        return output_image;
    }

    // Process interior pixels using direct row-pointer access.
    for (int row = 1; row < input_image.rows - 1; ++row)
    {
        const unsigned char* previous_row = input_image.ptr<unsigned char>(row - 1);
        const unsigned char* current_row = input_image.ptr<unsigned char>(row);
        const unsigned char* next_row = input_image.ptr<unsigned char>(row + 1);

        unsigned char* output_row = output_image.ptr<unsigned char>(row);

        for (int col = 1; col < input_image.cols - 1; ++col)
        {
            // Calculate horizontal and vertical gradients directly.
            const int gradient_x = (previous_row[col + 1] + 2 * current_row[col + 1] + next_row[col + 1]) -
                                   (previous_row[col - 1] + 2 * current_row[col - 1] + next_row[col - 1]);

            const int gradient_y = (next_row[col - 1] + 2 * next_row[col] + next_row[col + 1]) -
                                   (previous_row[col - 1] + 2 * previous_row[col] + previous_row[col + 1]);

            // Calculate gradient magnitude using the Euclidean norm.
            const double magnitude = std::sqrt(static_cast<double>(gradient_x) * gradient_x +
                                               static_cast<double>(gradient_y) * gradient_y);

            // Convert the gradient magnitude into a binary edge value.
            output_row[col] = magnitude >= threshold ? 255 : 0;
        }
    }

    return output_image;
}


} // namespace saber