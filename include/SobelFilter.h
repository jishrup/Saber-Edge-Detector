/**
 * @file SobelFilter.h
 * @brief Defines the interface for Sobel edge detection.
 */

#ifndef SABER_SOBEL_FILTER_H
#define SABER_SOBEL_FILTER_H

#include <opencv2/core.hpp>

#include <stdexcept>

namespace saber
{

/**
 * @brief Performs Sobel edge detection on grayscale images.
 *
 * Applies standard 3x3 Sobel kernels to calculate horizontal and
 * vertical gradients, producing a binary edge image.
 *
 * Border pixels are set to zero.
 */
class SobelFilter
{
public:
    /**
     * @brief Applies the naive Sobel edge detection algorithm.
     *
     * @param input_image Input 8-bit, single-channel grayscale image.
     * @param threshold Gradient magnitude threshold for edge detection.
     * @return Binary edge image with the same dimensions as the input.
     *
     * @throws std::invalid_argument If the input is empty, has an
     *         unsupported image type, or the threshold is invalid.
     *
     * @note Pixels with gradient magnitude greater than or equal to
     *       the threshold are set to 255. All others are set to zero.
     *       Border pixels are always zero.
     */
    static cv::Mat ApplyNaive(const cv::Mat& input_image, double threshold);

private:
    /**
     * @brief Standard 3x3 Sobel kernel for horizontal gradients.
     */
    inline static constexpr int SobelX[3][3] =  {
                                                  { -1,  0,  1 },
                                                  { -2,  0,  2 },
                                                  { -1,  0,  1 }
                                                };

    /**
     * @brief Standard 3x3 Sobel kernel for vertical gradients.
     */
    inline static constexpr int SobelY[3][3] =  {
                                                  { -1, -2, -1 },
                                                  {  0,  0,  0 },
                                                  {  1,  2,  1 }
                                                };
};

} // namespace saber

#endif // SABER_SOBEL_FILTER_H