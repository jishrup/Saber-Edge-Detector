
/**
 * @file benchmark_sobel.cpp
 * @brief Benchmarks naive, optimized, and OpenCV Sobel implementations.
 */

#include "SobelFilter.h"

#include <benchmark/benchmark.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <cmath>
#include <cstdint>

namespace
{

/**
 * @brief Creates a deterministic grayscale image for benchmarking.
 */
cv::Mat CreateInputImage(int width, int height)
{
    cv::Mat input_image(height, width, CV_8UC1);

    for (int row = 0; row < height; ++row)
    {
        unsigned char* image_row = input_image.ptr<unsigned char>(row);

        for (int col = 0; col < width; ++col)
        {
            image_row[col] = static_cast<unsigned char>((row * 17 + col * 31 + (row * col) % 251) % 256);
        }
    }

    return input_image;
}

/**
 * @brief Applies OpenCV Sobel with the same binary threshold semantics.
 */
cv::Mat ApplyOpenCvSobel(const cv::Mat& input_image, double threshold)
{
    cv::Mat gradient_x;
    cv::Mat gradient_y;

    cv::Sobel(input_image, gradient_x, CV_16S, 1, 0, 3);
    cv::Sobel(input_image, gradient_y, CV_16S, 0, 1, 3);

    cv::Mat output_image = cv::Mat::zeros(input_image.size(), CV_8UC1);

    for (int row = 1; row < input_image.rows - 1; ++row)
    {
        const short* gradient_x_row = gradient_x.ptr<short>(row);
        const short* gradient_y_row = gradient_y.ptr<short>(row);
        unsigned char* output_row = output_image.ptr<unsigned char>(row);

        for (int col = 1; col < input_image.cols - 1; ++col)
        {
            const double gx = gradient_x_row[col];
            const double gy = gradient_y_row[col];
            const double magnitude = std::sqrt(gx * gx + gy * gy);

            output_row[col] = magnitude >= threshold ? 255 : 0;
        }
    }

    return output_image;
}

/**
 * @brief Measures the naive Sobel implementation.
 */
void BenchmarkNaive(benchmark::State& state)
{
    const cv::Mat input_image = CreateInputImage(
        static_cast<int>(state.range(0)),
        static_cast<int>(state.range(1)));

    for (auto _ : state)
    {
        cv::Mat output_image = saber::SobelFilter::ApplyNaive(input_image, 100.0);
        benchmark::DoNotOptimize(output_image.data);
    }

    state.SetItemsProcessed(state.iterations() * input_image.total());
}

/**
 * @brief Measures the optimized Sobel implementation.
 */
void BenchmarkOptimized(benchmark::State& state)
{
    const cv::Mat input_image = CreateInputImage(static_cast<int>(state.range(0)), static_cast<int>(state.range(1)));

    for (auto _ : state)
    {
        cv::Mat output_image = saber::SobelFilter::ApplyOptimized(input_image, 100.0);
        benchmark::DoNotOptimize(output_image.data);
    }

    state.SetItemsProcessed(state.iterations() * input_image.total());
}

/**
 * @brief Measures the OpenCV-based Sobel reference.
 */
void BenchmarkOpenCv(benchmark::State& state)
{
    const cv::Mat input_image = CreateInputImage(static_cast<int>(state.range(0)), static_cast<int>(state.range(1)));

    for (auto _ : state)
    {
        cv::Mat output_image = ApplyOpenCvSobel(input_image, 100.0);
        benchmark::DoNotOptimize(output_image.data);
    }

    state.SetItemsProcessed(state.iterations() * input_image.total());
}


/**
 * @brief Benchmarks parallel Sobel with a configurable worker count.
 *
 * @param state Google Benchmark state containing image size and thread count.
 */
static void BenchmarkParallel(benchmark::State& state)
{
    const int width = static_cast<int>(state.range(0));
    const int height = static_cast<int>(state.range(1));
    const unsigned int thread_count = static_cast<unsigned int>(state.range(2));

    const cv::Mat input_image = CreateInputImage(width, height);

    for (auto _ : state)
    {
        cv::Mat output_image = saber::SobelFilter::ApplyParallel(input_image, 100.0, thread_count);

        benchmark::DoNotOptimize(output_image.data);
    }

    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(width) * height);
}


} // namespace

BENCHMARK(BenchmarkNaive)
    ->Args({640, 480})
    ->Args({1280, 720})
    ->Args({1920, 1080})
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BenchmarkOptimized)
    ->Args({640, 480})
    ->Args({1280, 720})
    ->Args({1920, 1080})
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BenchmarkParallel)
    ->UseRealTime()
    ->Args({640, 480, 1})
    ->Args({640, 480, 2})
    ->Args({640, 480, 4})
    ->Args({640, 480, 8})
    ->Args({1280, 720, 1})
    ->Args({1280, 720, 2})
    ->Args({1280, 720, 4})
    ->Args({1280, 720, 8})
    ->Args({1920, 1080, 1})
    ->Args({1920, 1080, 2})
    ->Args({1920, 1080, 4})
    ->Args({1920, 1080, 8});

BENCHMARK(BenchmarkOpenCv)
    ->Args({640, 480})
    ->Args({1280, 720})
    ->Args({1920, 1080})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
