
/**
 * @file test_cli.cpp
 * @brief Integration tests for the Saber Edge Detector command-line application.
 *
 * Verifies command-line arguments, threshold handling, image output,
 * and failure behavior by executing the compiled application.
 */

#include <gtest/gtest.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace
{

namespace fs = std::filesystem;

/**
 * @brief Stores the exit status and captured output of a CLI execution.
 */
struct CommandResult
{
    int exit_code;
    std::string output;
};

/**
 * @brief Executes the Sobel application and captures its console output.
 *
 * @param arguments Command-line arguments to pass to the application.
 * @param test_directory Directory used for captured console output.
 * @return Process exit status and combined standard output/error.
 */
CommandResult RunDetector(const std::string& arguments, const fs::path& test_directory)
{
    const fs::path log_path = test_directory / "command_output.txt";

    const std::string command = "\"" + std::string(SOBEL_DETECTOR_PATH) + "\" " + arguments + " > \"" + log_path.string() + "\" 2>&1";

    const int exit_code = std::system(command.c_str());

    std::ifstream log_file(log_path);
    const std::string output((std::istreambuf_iterator<char>(log_file)), std::istreambuf_iterator<char>());

    return {exit_code, output};
}

/**
 * @brief Creates an isolated temporary directory for each test.
 *
 * @return Path to the test-specific directory.
 */
fs::path CreateTestDirectory()
{
    const auto* test_info = ::testing::UnitTest::GetInstance()->current_test_info();

    const fs::path test_directory = fs::temp_directory_path() /
                                    "saber_edge_detector_tests" /
                                    test_info->test_suite_name() /
                                    test_info->name();

    std::error_code error;
    fs::remove_all(test_directory, error);

    if (error)
    {
        throw fs::filesystem_error("Failed to clean test directory", test_directory, error);
    }

    fs::create_directories(test_directory);

    return test_directory;
}

/**
 * @brief Verifies the output image matches the expected Sobel edge pattern.
 *
 * @param output_path Path to the generated image.
 */
void ExpectCorrectEdgeImage(const fs::path& output_path)
{
    ASSERT_TRUE(fs::exists(output_path));

    const cv::Mat output_image = cv::imread(output_path.string(), cv::IMREAD_GRAYSCALE);

    ASSERT_FALSE(output_image.empty());
    ASSERT_EQ(output_image.rows, 5);
    ASSERT_EQ(output_image.cols, 5);
    ASSERT_EQ(output_image.type(), CV_8UC1);

    const cv::Mat expected_image = cv::Mat_<unsigned char>({5, 5}, { 0, 0, 0, 0, 0,
                                                                     0, 0, 255, 255, 0,
                                                                     0, 0, 255, 255, 0,
                                                                     0, 0, 255, 255, 0,
                                                                     0, 0, 0, 0, 0});

    for (int row = 0; row < output_image.rows; ++row)
    {
        for (int col = 0; col < output_image.cols; ++col)
        {
            EXPECT_EQ(output_image.at<unsigned char>(row, col), expected_image.at<unsigned char>(row, col)) << "Pixel mismatch at (" << row << ", " << col << ")";
        }
    }
}

} // namespace

/**
 * @brief Verifies the application accepts the default threshold.
 */
TEST(SobelCliTest, UsesDefaultThreshold)
{
    const fs::path test_directory = CreateTestDirectory();
    const fs::path output_path = test_directory / "default.png";

    const std::string arguments = "\"" + std::string(SOBEL_SAMPLE_PATH) + "\" \"" + output_path.string() + "\"";

    const CommandResult result = RunDetector(arguments, test_directory);

    ASSERT_EQ(result.exit_code, 0) << result.output;
    EXPECT_NE(result.output.find("Threshold: 100"), std::string::npos);

    ExpectCorrectEdgeImage(output_path);
}

/**
 * @brief Verifies the application accepts a custom threshold.
 */
TEST(SobelCliTest, AcceptsCustomThreshold)
{
    const fs::path test_directory = CreateTestDirectory();
    const fs::path output_path = test_directory / "custom.png";

    const std::string arguments = "\"" + std::string(SOBEL_SAMPLE_PATH) + "\" \"" + output_path.string() + "\" 150";

    const CommandResult result = RunDetector(arguments, test_directory);

    ASSERT_EQ(result.exit_code, 0) << result.output;
    EXPECT_NE(result.output.find("Threshold: 150"), std::string::npos);

    ExpectCorrectEdgeImage(output_path);
}

/**
 * @brief Verifies the application rejects a negative threshold.
 */
TEST(SobelCliTest, RejectsNegativeThreshold)
{
    const fs::path test_directory = CreateTestDirectory();
    const fs::path output_path = test_directory / "negative.png";

    const std::string arguments = "\"" + std::string(SOBEL_SAMPLE_PATH) + "\" \"" + output_path.string() + "\" -10";

    const CommandResult result = RunDetector(arguments, test_directory);

    EXPECT_NE(result.exit_code, 0);
    EXPECT_NE(result.output.find("Invalid threshold:"), std::string::npos);
    EXPECT_FALSE(fs::exists(output_path));
}

/**
 * @brief Verifies the application rejects a nonnumeric threshold.
 */
TEST(SobelCliTest, RejectsNonnumericThreshold)
{
    const fs::path test_directory = CreateTestDirectory();
    const fs::path output_path = test_directory / "nonnumeric.png";

    const std::string arguments = "\"" + std::string(SOBEL_SAMPLE_PATH) + "\" \"" + output_path.string() + "\" abc";

    const CommandResult result = RunDetector(arguments, test_directory);

    EXPECT_NE(result.exit_code, 0);
    EXPECT_NE(result.output.find("Invalid threshold:"), std::string::npos);
    EXPECT_FALSE(fs::exists(output_path));
}

/**
 * @brief Verifies the application rejects trailing threshold characters.
 */
TEST(SobelCliTest, RejectsMalformedThreshold)
{
    const fs::path test_directory = CreateTestDirectory();
    const fs::path output_path = test_directory / "malformed.png";

    const std::string arguments = "\"" + std::string(SOBEL_SAMPLE_PATH) + "\" \"" + output_path.string() + "\" 100abc";

    const CommandResult result = RunDetector(arguments, test_directory);

    EXPECT_NE(result.exit_code, 0);
    EXPECT_NE(result.output.find("Invalid threshold:"), std::string::npos);
    EXPECT_FALSE(fs::exists(output_path));
}

/**
 * @brief Verifies the application rejects non-finite thresholds.
 */
TEST(SobelCliTest, RejectsNonFiniteThreshold)
{
    const fs::path test_directory = CreateTestDirectory();
    const fs::path output_path = test_directory / "nonfinite.png";

    for (const std::string threshold : {"nan", "inf", "-inf"})
    {
        const std::string arguments = "\"" + std::string(SOBEL_SAMPLE_PATH) + "\" \"" + output_path.string() + "\" " + threshold;

        const CommandResult result = RunDetector(arguments, test_directory);

        EXPECT_NE(result.exit_code, 0) << threshold;
        EXPECT_NE(result.output.find("Invalid threshold:"), std::string::npos);
        EXPECT_FALSE(fs::exists(output_path));
    }
}

/**
 * @brief Verifies the application rejects missing command-line arguments.
 */
TEST(SobelCliTest, RejectsMissingArguments)
{
    const fs::path test_directory = CreateTestDirectory();

    const CommandResult result = RunDetector("", test_directory);

    EXPECT_NE(result.exit_code, 0);
    EXPECT_NE(result.output.find("Usage:"), std::string::npos);
}

/**
 * @brief Verifies the application rejects excessive command-line arguments.
 */
TEST(SobelCliTest, RejectsTooManyArguments)
{
    const fs::path test_directory = CreateTestDirectory();

    const CommandResult result = RunDetector("input.png output.png 100 extra", test_directory);

    EXPECT_NE(result.exit_code, 0);
    EXPECT_NE(result.output.find("Usage:"), std::string::npos);
}

/**
 * @brief Verifies the application reports a missing input image.
 */
TEST(SobelCliTest, RejectsMissingInputImage)
{
    const fs::path test_directory = CreateTestDirectory();
    const fs::path input_path = test_directory / "missing.png";
    const fs::path output_path = test_directory / "output.png";

    const std::string arguments = "\"" + input_path.string() + "\" \"" + output_path.string() + "\"";

    const CommandResult result = RunDetector(arguments, test_directory);

    EXPECT_NE(result.exit_code, 0);
    EXPECT_NE(result.output.find("Failed to load image:"), std::string::npos);
    EXPECT_FALSE(fs::exists(output_path));
}

/**
 * @brief Verifies the application reports an invalid output destination.
 */
TEST(SobelCliTest, RejectsInvalidOutputPath)
{
    const fs::path test_directory = CreateTestDirectory();
    const fs::path output_path = test_directory / "missing_directory" / "output.png";

    const std::string arguments = "\"" + std::string(SOBEL_SAMPLE_PATH) + "\" \"" + output_path.string() + "\"";

    const CommandResult result = RunDetector(arguments, test_directory);

    EXPECT_NE(result.exit_code, 0);
    EXPECT_NE(result.output.find("Failed to save image:"), std::string::npos);
    EXPECT_FALSE(fs::exists(output_path));
}
