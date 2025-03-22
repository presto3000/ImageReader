
//Copyright 2022 Chris Pawłowski

#include "CImg.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <chrono>
#include <thread>
#include <future>
#include <algorithm>
#include <filesystem>

#define cimg_use_jpeg


using namespace cimg_library;
namespace fs = std::filesystem;

// Progress bar function
void ShowProgressBar(const std::string& taskName, int progress, int total) {
    int barWidth = 50; // Width of the progress bar
    float percentage = static_cast<float>(progress) / total;
    int pos = static_cast<int>(barWidth * percentage);

    std::cout << "\r" << taskName << " [";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(percentage * 100.0) << "%";
    std::cout.flush();
    if (progress == total) std::cout << std::endl;
}

class PPMImage {
public:
    struct RGB {
        unsigned char r, g, b;
        float luminance;
        int x, y;
    };

    ~PPMImage() = default;
    PPMImage() = default;
    
    void Save(const std::string& filename);
    void Read(const std::string& filename);
    void Resize(int newHeight, int newWidth);
    void ComputeLuminanceAndSort();
    void UpdatePixels(PPMImage* source, PPMImage* target);
    void ApplyUpdatedPixels();
    void CountUniqueColors();

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

private:
    int width = 0, height = 0;
    std::string version = "P6";
    std::vector<std::vector<RGB>> imageData;
    std::vector<RGB> sortedPixels;
    std::map<std::pair<int, int>, RGB> pixelMap;

    void AllocateImage();
};

void PPMImage::Save(const std::string& filename) {
    std::ofstream output(filename, std::ios::binary);
    if (!output) return;

    output << version << "\n" << width << " " << height << "\n255\n";
    if (version == "P6") {
        for (const auto& row : imageData) {
            for (const auto& pixel : row) {
                output.write(reinterpret_cast<const char*>(&pixel), 3);
            }
        }
    }
    output.close();
}

void PPMImage::Read(const std::string& filename) {
    std::ifstream input(filename, std::ios::binary);
    if (!input) return;
    
    input >> version >> width >> height;
    int maxVal;
    input >> maxVal;
    input.ignore();
    
    AllocateImage();

    if (version == "P6") {
        for (auto& row : imageData) {
            for (auto& pixel : row) {
                input.read(reinterpret_cast<char*>(&pixel), 3);
            }
        }
    }
    input.close();
}

void PPMImage::AllocateImage() {
    imageData.resize(height, std::vector<RGB>(width, {255, 255, 255, 0, 0}));
}

void PPMImage::Resize(int newHeight, int newWidth) {
    std::vector<std::vector<RGB>> resized(newHeight, std::vector<RGB>(newWidth));
    for (int i = 0; i < newHeight; ++i) {
        for (int j = 0; j < newWidth; ++j) {
            resized[i][j] = imageData[i * height / newHeight][j * width / newWidth];
        }
    }
    imageData = std::move(resized);
    height = newHeight;
    width = newWidth;
}

void PPMImage::ComputeLuminanceAndSort() {
    sortedPixels.clear();
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            auto& pixel = imageData[i][j];
            pixel.luminance = 0.299f * pixel.r + 0.587f * pixel.g + 0.114f * pixel.b;
            pixel.x = j;
            pixel.y = i;
            sortedPixels.push_back(pixel);
        }
    }
    std::stable_sort(sortedPixels.begin(), sortedPixels.end(), [](const RGB& a, const RGB& b) {
        return a.luminance < b.luminance;
    });
}

void PPMImage::UpdatePixels(PPMImage* source, PPMImage* target) {
    if (!source || !target) return;

    for (size_t i = 0; i < source->sortedPixels.size(); ++i) {
        RGB& srcPixel = source->sortedPixels[i];
        RGB& tgtPixel = target->sortedPixels[i];
        
        pixelMap[{tgtPixel.x, tgtPixel.y}] = {srcPixel.r, srcPixel.g, srcPixel.b};
    }
}

void PPMImage::ApplyUpdatedPixels() {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (pixelMap.count({j, i})) {
                imageData[i][j] = pixelMap[{j, i}];
            }
        }
    }
}

void PPMImage::CountUniqueColors() {
    std::set<std::tuple<unsigned char, unsigned char, unsigned char>> uniqueColors;
    for (const auto& row : imageData) {
        for (const auto& pixel : row) {
            uniqueColors.insert({pixel.r, pixel.g, pixel.b});
        }
    }
    std::cout << "Unique colors: " << uniqueColors.size() << std::endl;
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    
    printf("====== IMAGE PAINTER 0.1 ======\n");
    printf("In Solution directory we have Picture A and B.\nProgram creates Picture C using Picture B as a base with picture's A colors\n");
    
    //========================================================
    // Load JPEG image
    fs::path currentPath = fs::current_path(); // Get the current working directory
    fs::path imagePathA = currentPath / "obrazA.jpg"; // Append the image file name for obrazA
    fs::path imagePathB = currentPath / "obrazB.jpg"; // Append the image file name for obrazB
    
    std::cout << "Image path A: " << imagePathA << std::endl;
    std::cout << "Image path B: " << imagePathB << std::endl;

    // Progress bar for loading images
    ShowProgressBar("Loading Images", 0, 3);

    try {
        if (!fs::exists(imagePathA)) {
            throw std::runtime_error("File 'obrazA.jpg' not found in the current directory.");
        }
        CImg<unsigned char> imA(imagePathA.string().c_str()); // Load the image
        printf("obrazA Loaded\n");
        // Save as PPMImage
        imA.save("A.ppm");
        printf("obrazA Saved as A.ppm\n");
        ShowProgressBar("Loading Images", 1, 3);
    } catch (const CImgIOException& e) {
        std::cerr << "Error loading obrazA: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    try {
        if (!fs::exists(imagePathB)) {
            throw std::runtime_error("File 'obrazB.jpg' not found in the current directory.");
        }
        CImg<unsigned char> imB(imagePathB.string().c_str()); // Load the image
        printf("obrazB Loaded\n");
        // Save as PPMImage
        imB.save("B.ppm");
        printf("obrazB Saved as B.ppm\n");
        ShowProgressBar("Loading Images", 2, 3);
    } catch (const CImgIOException& e) {
        std::cerr << "Error loading obrazB: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    ShowProgressBar("Loading Images", 3, 3);

    PPMImage imgA, imgB;
    imgA.Read("A.ppm");
    imgB.Read("B.ppm");

    if (imgA.GetHeight() != imgB.GetHeight() || imgA.GetWidth() != imgB.GetWidth()) {
        imgB.Resize(imgA.GetHeight(), imgA.GetWidth());
    }

    // Progress bar for processing images
    ShowProgressBar("Processing Images", 0, 4);

    auto task1 = std::async(std::launch::async, &PPMImage::ComputeLuminanceAndSort, &imgA);
    auto task2 = std::async(std::launch::async, &PPMImage::ComputeLuminanceAndSort, &imgB);
    task1.get();
    ShowProgressBar("Processing Images", 1, 4);
    task2.get();
    ShowProgressBar("Processing Images", 2, 4);

    auto task3 = std::async(std::launch::async, &PPMImage::CountUniqueColors, &imgA);
    auto task4 = std::async(std::launch::async, &PPMImage::CountUniqueColors, &imgB);
    task3.get();
    ShowProgressBar("Processing Images", 3, 4);
    task4.get();
    ShowProgressBar("Processing Images", 4, 4);

    imgB.UpdatePixels(&imgA, &imgB);
    imgB.ApplyUpdatedPixels();

    imgA.Save("ResultA.ppm");
    imgB.Save("ResultB.ppm");

    
    // Load PPMImage image
    CImg<unsigned char> PPMImageb("ResultB.ppm");
    // Save as 
    PPMImageb.save("C.png");
    printf("====== C.png Saved ======\n");
    //========================================================

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Execution time: " << std::chrono::duration<float, std::milli>(end - start).count() << " ms" << std::endl;



    return 0;
}