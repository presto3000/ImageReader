#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

cv::Mat matchHistograms(const cv::Mat& src, const cv::Mat& target) {
    // Resize target to match the size of src
    cv::Mat resizedTarget;
    cv::resize(target, resizedTarget, src.size());

    cv::Mat srcLab, targetLab;
    cv::cvtColor(src, srcLab, cv::COLOR_BGR2Lab);
    cv::cvtColor(resizedTarget, targetLab, cv::COLOR_BGR2Lab);

    // Split the channels (L, a, b)
    std::vector<cv::Mat> srcChannels, targetChannels;
    cv::split(srcLab, srcChannels);
    cv::split(targetLab, targetChannels);

    cv::Mat srcL = srcChannels[0];
    cv::Mat srcA = srcChannels[1];
    cv::Mat srcB = srcChannels[2];

    cv::Mat targetL = targetChannels[0];
    cv::Mat targetA = targetChannels[1];
    cv::Mat targetB = targetChannels[2];

    // Apply histogram matching for the 'a' and 'b' channels
    cv::Mat matchedA, matchedB;

    // Normalize the channels
    cv::normalize(srcA, srcA, 0, 255, cv::NORM_MINMAX);
    cv::normalize(targetA, targetA, 0, 255, cv::NORM_MINMAX);
    cv::normalize(srcB, srcB, 0, 255, cv::NORM_MINMAX);
    cv::normalize(targetB, targetB, 0, 255, cv::NORM_MINMAX);

    // Perform histogram matching manually (for example, using histogram equalization)
    cv::equalizeHist(srcA, matchedA);
    cv::equalizeHist(srcB, matchedB);

    // For the 'L' channel, just use the original 'srcL'
    cv::Mat resultL = srcL; // L channel remains the same

    // For the 'a' and 'b' channels, we have to apply histogram matching as we did with the 'a' channel
    cv::Mat resultA = matchedA; // Apply histogram matching for the 'a' channel
    cv::Mat resultB = matchedB; // Apply histogram matching for the 'b' channel

    // Merge the result channels (L, a, b) into the final result
    std::vector<cv::Mat> resultChannels = { resultL, resultA, resultB };
    cv::Mat matchedImage;
    cv::merge(resultChannels, matchedImage);

    // Convert back to BGR color space
    cv::Mat finalImage;
    cv::cvtColor(matchedImage, finalImage, cv::COLOR_Lab2BGR);

    return finalImage;
}

cv::Mat colorTransfer(const cv::Mat& src, const cv::Mat& target) {
    // Convert images to Lab color space
    cv::Mat srcLab, targetLab;
    cv::cvtColor(src, srcLab, cv::COLOR_BGR2Lab);
    cv::cvtColor(target, targetLab, cv::COLOR_BGR2Lab);

    // Split the channels (L, a, b)
    std::vector<cv::Mat> srcChannels, targetChannels;
    cv::split(srcLab, srcChannels);
    cv::split(targetLab, targetChannels);

    cv::Mat srcL = srcChannels[0];
    cv::Mat srcA = srcChannels[1];
    cv::Mat srcB = srcChannels[2];

    cv::Mat targetL = targetChannels[0];
    cv::Mat targetA = targetChannels[1];
    cv::Mat targetB = targetChannels[2];

    // Function to match the mean and standard deviation of one channel to another
    auto matchMeanStd = [](cv::Mat& src, cv::Mat& target) {
        // Calculate the mean and std of the source and target
        cv::Scalar meanSrc, stddevSrc;
        cv::Scalar meanTarget, stddevTarget;

        cv::meanStdDev(src, meanSrc, stddevSrc);
        cv::meanStdDev(target, meanTarget, stddevTarget);

        // Normalize the source image based on the target mean and std
        cv::Mat result;
        result = (src - meanSrc[0]) * (stddevTarget[0] / stddevSrc[0]) + meanTarget[0];
        result = cv::max(result, 0);  // Ensure the values are non-negative
        result = cv::min(result, 255); // Ensure the values do not exceed 255
        return result;
    };

    // Match the mean and standard deviation for the 'a' and 'b' channels
    cv::Mat resultA = matchMeanStd(srcA, targetA);
    cv::Mat resultB = matchMeanStd(srcB, targetB);

    // The L channel is unchanged
    cv::Mat resultL = srcL;

    // Merge the result channels (L, a, b) into the final result
    std::vector<cv::Mat> resultChannels = { resultL, resultA, resultB };
    cv::Mat resultLab;
    cv::merge(resultChannels, resultLab);

    // Convert back to BGR color space
    cv::Mat finalImage;
    cv::cvtColor(resultLab, finalImage, cv::COLOR_Lab2BGR);

    return finalImage;
}

int main() {
    // Image paths
    fs::path currentPath = fs::current_path(); // Get the current working directory
    fs::path imagePathA = currentPath / "obrazB.jpg"; // Append the image file name for obrazA
    fs::path imagePathB = currentPath / "obrazA.jpg"; // Append the image file name for obrazB

    // Load images
    cv::Mat imgA = cv::imread(imagePathA.string());
    cv::Mat imgB = cv::imread(imagePathB.string());

    if (imgA.empty()) {
        std::cerr << "Error: Could not open or find image A!" << std::endl;
        return -1;
    }

    if (imgB.empty()) {
        std::cerr << "Error: Could not open or find image B!" << std::endl;
        return -1;
    }

    std::cout << "Loaded images: " << imagePathA << " and " << imagePathB << std::endl;

    // Perform histogram matching
    cv::Mat resultImage = colorTransfer(imgA, imgB);

    // Save the result as a new image
    cv::imwrite("result_image.png", resultImage);

    std::cout << "Result image saved as result_image.png" << std::endl;

    return 0;
}