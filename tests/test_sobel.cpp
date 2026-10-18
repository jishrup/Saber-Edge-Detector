/**
 * @file test_sobel.cpp
 * @brief Unit tests for the naive Sobel edge detection algorithm.
 */

#include "SobelFilter.h"

#include <gtest/gtest.h>
#include <opencv2/core.hpp>

#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{

/**
 * @brief Verifies that two grayscale images contain identical pixels.
 *
 * @param actual Actual output image.
 * @param expected Expected output image.
 */
void ExpectImagesEqual(const cv::Mat& actual, const cv::Mat& expected)
{
    ASSERT_EQ(actual.size(), expected.size());
    ASSERT_EQ(actual.type(), expected.type());

    for (int row = 0; row < actual.rows; ++row)
    {
        for (int col = 0; col < actual.cols; ++col)
        {
            EXPECT_EQ(actual.at<unsigned char>(row, col), expected.at<unsigned char>(row, col)) << "Pixel mismatch at (" << row << ", " << col << ")";
        }
    }
}

/**
 * @brief Compares the optimized Sobel output against the naive implementation.
 */
void ExpectOptimizedMatchesNaive(const cv::Mat& input_image, double threshold)
{
    const cv::Mat expected_image = saber::SobelFilter::ApplyNaive(input_image, threshold);
    const cv::Mat actual_image = saber::SobelFilter::ApplyOptimized(input_image, threshold);

    ASSERT_EQ(actual_image.size(), expected_image.size());
    ASSERT_EQ(actual_image.type(), expected_image.type());

    for (int row = 0; row < expected_image.rows; ++row)
    {
        for (int col = 0; col < expected_image.cols; ++col)
        {
            EXPECT_EQ(actual_image.at<unsigned char>(row, col), expected_image.at<unsigned char>(row, col)) << "Mismatch at row " << row << ", column " << col;
        }
    }
}

} // namespace

/**
 * @brief Verifies vertical edge detection.
 */
TEST(SobelFilterTest, DetectsVerticalEdge)
{
    const cv::Mat input_image = cv::Mat_<unsigned char>({5, 5}, {20, 20, 20, 220, 220,
                                                                 20, 20, 20, 220, 220,
                                                                 20, 20, 20, 220, 220,
                                                                 20, 20, 20, 220, 220,
                                                                 20, 20, 20, 220, 220});

    const cv::Mat expected_image = cv::Mat_<unsigned char>({5, 5}, {0, 0, 0, 0, 0,
                                                                    0, 0, 255, 255, 0,
                                                                    0, 0, 255, 255, 0,
                                                                    0, 0, 255, 255, 0,
                                                                    0, 0, 0, 0, 0});

    const cv::Mat output_image = saber::SobelFilter::ApplyNaive(input_image, 100.0);

    ExpectImagesEqual(output_image, expected_image);
}

/**
 * @brief Verifies horizontal edge detection.
 */
TEST(SobelFilterTest, DetectsHorizontalEdge)
{
    const cv::Mat input_image = cv::Mat_<unsigned char>({5, 5}, {20, 20, 20, 20, 20,
                                                                 20, 20, 20, 20, 20,
                                                                 20, 20, 20, 20, 20,
                                                                 220, 220, 220, 220, 220,
                                                                 220, 220, 220, 220, 220});

    const cv::Mat expected_image = cv::Mat_<unsigned char>({5, 5}, {0, 0, 0, 0, 0,
                                                                    0, 0, 0, 0, 0,
                                                                    0, 255, 255, 255, 0,
                                                                    0, 255, 255, 255, 0,
                                                                    0, 0, 0, 0, 0});

    const cv::Mat output_image = saber::SobelFilter::ApplyNaive(input_image, 100.0);

    ExpectImagesEqual(output_image, expected_image);
}

/**
 * @brief Verifies that a constant-intensity image produces no edges.
 */
TEST(SobelFilterTest, UniformImageProducesNoEdges)
{
    const cv::Mat input_image(5, 5, CV_8UC1, cv::Scalar(128));
    const cv::Mat expected_image = cv::Mat::zeros(5, 5, CV_8UC1);

    const cv::Mat output_image = saber::SobelFilter::ApplyNaive(input_image, 100.0);

    ExpectImagesEqual(output_image, expected_image);
}

/**
 * @brief Verifies that images smaller than 3x3 produce no edges.
 */
TEST(SobelFilterTest, SmallImageProducesNoEdges)
{
    const cv::Mat input_image(2, 2, CV_8UC1, cv::Scalar(255));
    const cv::Mat expected_image = cv::Mat::zeros(2, 2, CV_8UC1);

    const cv::Mat output_image = saber::SobelFilter::ApplyNaive(input_image, 100.0);

    ExpectImagesEqual(output_image, expected_image);
}

/**
 * @brief Verifies that empty input images are rejected.
 */
TEST(SobelFilterTest, RejectsEmptyImage)
{
    const cv::Mat input_image;

    EXPECT_THROW(saber::SobelFilter::ApplyNaive(input_image, 100.0), std::invalid_argument);
}

/**
 * @brief Verifies that unsupported image types are rejected.
 */
TEST(SobelFilterTest, RejectsInvalidImageType)
{
    const cv::Mat input_image(5, 5, CV_8UC3, cv::Scalar(20, 20, 20));

    EXPECT_THROW(saber::SobelFilter::ApplyNaive(input_image, 100.0), std::invalid_argument);
}

/**
 * @brief Verifies that invalid gradient thresholds are rejected.
 */
TEST(SobelFilterTest, RejectsInvalidThreshold)
{
    const cv::Mat input_image(5, 5, CV_8UC1, cv::Scalar(128));

    EXPECT_THROW(saber::SobelFilter::ApplyNaive(input_image, -1.0), std::invalid_argument);
    EXPECT_THROW(saber::SobelFilter::ApplyNaive(input_image, std::numeric_limits<double>::quiet_NaN()), std::invalid_argument);
    EXPECT_THROW(saber::SobelFilter::ApplyNaive(input_image, std::numeric_limits<double>::infinity()), std::invalid_argument);
}


/**
 * @brief Verifies optimized Sobel detection against known edge patterns.
 */
TEST(SobelOptimizationTest, MatchesNaiveForKnownEdges)
{
    const cv::Mat vertical_edge = cv::Mat_<unsigned char>({5, 5}, {20, 20, 20, 220, 220,
                                                                   20, 20, 20, 220, 220,
                                                                   20, 20, 20, 220, 220,
                                                                   20, 20, 20, 220, 220,
                                                                   20, 20, 20, 220, 220});

    const cv::Mat horizontal_edge = cv::Mat_<unsigned char>({5, 5}, {20, 20, 20, 20, 20,
                                                                     20, 20, 20, 20, 20,
                                                                     20, 20, 20, 20, 20,
                                                                     220, 220, 220, 220, 220,
                                                                     220, 220, 220, 220, 220});

    ExpectOptimizedMatchesNaive(vertical_edge, 100.0);
    ExpectOptimizedMatchesNaive(horizontal_edge, 100.0);
}

/**
 * @brief Verifies optimized Sobel detection on deterministic random images.
 */
TEST(SobelOptimizationTest, MatchesNaiveForRandomImages)
{
    std::mt19937 random_generator(42);
    std::uniform_int_distribution<int> pixel_distribution(0, 255);

    const std::vector<cv::Size> image_sizes = {{3, 3},
                                               {17, 13},
                                               {128, 96},
                                               {640, 480}};

    const std::vector<double> thresholds = {0.0, 50.0, 100.0, 255.0, 500.0, 1500.0};

    for (const cv::Size& image_size : image_sizes)
    {
        cv::Mat input_image(image_size, CV_8UC1);

        for (int row = 0; row < input_image.rows; ++row)
        {
            for (int col = 0; col < input_image.cols; ++col)
            {
                input_image.at<unsigned char>(row, col) = static_cast<unsigned char>(pixel_distribution(random_generator));
            }
        }

        for (double threshold : thresholds)
        {
            SCOPED_TRACE("Image size: " + std::to_string(image_size.width) + "x" +
                         std::to_string(image_size.height) + ", threshold: " +
                         std::to_string(threshold));

            ExpectOptimizedMatchesNaive(input_image, threshold);
        }
    }
}

/**
 * @brief Verifies optimized Sobel detection near threshold boundaries.
 */
TEST(SobelOptimizationTest, MatchesNaiveForThresholdBoundaries)
{
    const cv::Mat input_image = cv::Mat_<unsigned char>({3, 3}, {0, 0, 0,
                                                                 0, 0, 100,
                                                                 0, 0, 0});

    const std::vector<double> thresholds = {0.0,
                                            100.0,
                                            199.0,
                                            200.0,
                                            201.0,
                                            std::nextafter(200.0, 0.0),
                                            std::nextafter(200.0, std::numeric_limits<double>::infinity())};

    for (double threshold : thresholds)
    {
        ExpectOptimizedMatchesNaive(input_image, threshold);
    }
}

/**
 * @brief Verifies optimized Sobel behavior on undersized images.
 */
TEST(SobelOptimizationTest, MatchesNaiveForSmallImages)
{
    const std::vector<cv::Size> image_sizes = {{1, 1},
                                               {2, 2},
                                               {2, 5},
                                               {5, 2}};

    for (const cv::Size& image_size : image_sizes)
    {
        const cv::Mat input_image(image_size, CV_8UC1, cv::Scalar(255));

        ExpectOptimizedMatchesNaive(input_image, 100.0);
    }
}

/**
 * @brief Verifies optimized Sobel detection on non-contiguous image regions.
 */
TEST(SobelOptimizationTest, MatchesNaiveForNonContiguousImages)
{
    cv::Mat parent_image(20, 30, CV_8UC1);

    for (int row = 0; row < parent_image.rows; ++row)
    {
        for (int col = 0; col < parent_image.cols; ++col)
        {
            parent_image.at<unsigned char>(row, col) = static_cast<unsigned char>((row * 17 + col * 11) % 256);
        }
    }

    const cv::Mat input_image = parent_image(cv::Rect(4, 3, 15, 12));

    ASSERT_FALSE(input_image.isContinuous());

    ExpectOptimizedMatchesNaive(input_image, 100.0);
    ExpectOptimizedMatchesNaive(input_image, 255.0);
}

/**
 * @brief Verifies optimized Sobel rejects empty images.
 */
TEST(SobelOptimizationTest, RejectsEmptyImage)
{
    const cv::Mat input_image;

    EXPECT_THROW(saber::SobelFilter::ApplyOptimized(input_image, 100.0), std::invalid_argument);
}

/**
 * @brief Verifies optimized Sobel rejects unsupported image types.
 */
TEST(SobelOptimizationTest, RejectsInvalidImageType)
{
    const cv::Mat input_image(5, 5, CV_8UC3, cv::Scalar(0, 0, 0));

    EXPECT_THROW(saber::SobelFilter::ApplyOptimized(input_image, 100.0), std::invalid_argument);
}

/**
 * @brief Verifies optimized Sobel rejects invalid thresholds.
 */
TEST(SobelOptimizationTest, RejectsInvalidThreshold)
{
    const cv::Mat input_image(5, 5, CV_8UC1, cv::Scalar(128));

    EXPECT_THROW(saber::SobelFilter::ApplyOptimized(input_image, -1.0), std::invalid_argument);
    EXPECT_THROW(saber::SobelFilter::ApplyOptimized(input_image, std::numeric_limits<double>::quiet_NaN()), std::invalid_argument);
    EXPECT_THROW(saber::SobelFilter::ApplyOptimized(input_image, std::numeric_limits<double>::infinity()), std::invalid_argument);
}
