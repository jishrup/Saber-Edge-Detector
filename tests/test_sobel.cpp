/**
 * @file test_sobel.cpp
 * @brief Unit tests for the naive Sobel edge detection algorithm.
 */

#include "SobelFilter.h"

#include <gtest/gtest.h>
#include <opencv2/core.hpp>

#include <limits>
#include <stdexcept>

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

} // namespace

/**
 * @brief Verifies vertical edge detection.
 */
TEST(SobelFilterTest, DetectsVerticalEdge)
{
    const cv::Mat input_image = cv::Mat_<unsigned char>({5, 5}, { 20, 20, 20, 220, 220,
                                                                  20, 20, 20, 220, 220,
                                                                  20, 20, 20, 220, 220,
                                                                  20, 20, 20, 220, 220,
                                                                  20, 20, 20, 220, 220});

    const cv::Mat expected_image = cv::Mat_<unsigned char>({5, 5}, { 0, 0, 0, 0, 0,
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
    const cv::Mat input_image = cv::Mat_<unsigned char>({5, 5}, { 20, 20, 20, 20, 20,
                                                                  20, 20, 20, 20, 20,
                                                                  20, 20, 20, 20, 20,
                                                                  220, 220, 220, 220, 220,
                                                                  220, 220, 220, 220, 220});

    const cv::Mat expected_image = cv::Mat_<unsigned char>({5, 5}, { 0, 0, 0, 0, 0,
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