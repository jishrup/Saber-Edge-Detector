/**
 * @file SobelFilter.cpp
 * @brief Implements Sobel edge detection algorithms.
 */

#include "SobelFilter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <thread>
#include <vector>

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



cv::Mat SobelFilter::ApplyParallel(const cv::Mat& input_image,
                                   double threshold,
                                   unsigned int thread_count)
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

    const unsigned int interior_rows = static_cast<unsigned int>(input_image.rows - 2);

    // Select a conservative worker count when none is specified.
    if (thread_count == 0)
    {
        const unsigned int hardware_threads = std::thread::hardware_concurrency();
        const unsigned int available_threads = hardware_threads == 0 ? 1 : hardware_threads;

        thread_count = std::min(available_threads, 4u);

        // Avoid thread creation overhead for smaller images.
        if (input_image.total() < 640u * 480u)
        {
            thread_count = 1;
        }
    }

    // Never create more workers than there are interior rows.
    thread_count = std::min(thread_count, interior_rows);

    // Each worker writes to a distinct, half-open range of output rows.
    const auto process_rows = [&](int start_row, int end_row)
    {
        for (int row = start_row; row < end_row; ++row)
        {
            const unsigned char* previous_row = input_image.ptr<unsigned char>(row - 1);
            const unsigned char* current_row = input_image.ptr<unsigned char>(row);
            const unsigned char* next_row = input_image.ptr<unsigned char>(row + 1);

            unsigned char* output_row = output_image.ptr<unsigned char>(row);

            for (int col = 1; col < input_image.cols - 1; ++col)
            {
                const int gradient_x = (previous_row[col + 1] + 2 * current_row[col + 1] + next_row[col + 1]) -
                                       (previous_row[col - 1] + 2 * current_row[col - 1] + next_row[col - 1]);

                const int gradient_y = (next_row[col - 1] + 2 * next_row[col] + next_row[col + 1]) -
                                       (previous_row[col - 1] + 2 * previous_row[col] + previous_row[col + 1]);

                const double magnitude = std::sqrt(static_cast<double>(gradient_x) * gradient_x +
                                                   static_cast<double>(gradient_y) * gradient_y);

                output_row[col] = magnitude >= threshold ? 255 : 0;
            }
        }
    };

    // Run sequentially when only one worker is required.
    if (thread_count == 1)
    {
        process_rows(1, input_image.rows - 1);
        return output_image;
    }

    std::vector<std::thread> workers;
    workers.reserve(thread_count);

    // Partition interior rows into balanced, contiguous blocks.
    try
    {
        for (unsigned int worker_index = 0; worker_index < thread_count; ++worker_index)
        {
            const int start_row = 1 + static_cast<int>(static_cast<std::uint64_t>(worker_index) * interior_rows / thread_count);

            const int end_row = 1 + static_cast<int>(static_cast<std::uint64_t>(worker_index + 1) * interior_rows / thread_count);

            workers.emplace_back(process_rows, start_row, end_row);
        }
    }
    catch (...)
    {
        // Join already-started threads before propagating creation failures.
        for (std::thread& worker : workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }

        throw;
    }

    // All output writes must finish before returning the image.
    for (std::thread& worker : workers)
    {
        worker.join();
    }

    return output_image;
}

} // namespace saber