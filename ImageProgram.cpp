//Copyright 2022 Chris Pawłowski

#include "CImg.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <vector>
#include <stdio.h>
#include <string>
#include <map>
#include <set>
#include <chrono>
#include <thread>
#include <future>

#define sleep(x) Sleep(1000 * (x))
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWidth 60
#define TurnOnOffCountingPixels 1

using namespace cimg_library;

void UpdateBar(int PercentDone, int MaxValue);

class PPMImage
{
public:
	~PPMImage();
	PPMImage() {}
	
	struct RGB
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;

		float luminanceValue;
		int x;
		int y;
	};
	// Getters:
	int GetWidth() const { return Width; }
	int GetHeight() const { return Height; }

	// Save / Read
	void Save(std::string FileName);
	void Read(std::string FileName);

	// Resize
	void Resize(int Height, int Width);
	// Delete Image
	void DeleteImage();

	RGB **ImagePtr = nullptr;

	//std::vector<RGB> PixelsArray;
	std::vector<RGB> LumPosPixelsArray;

	std::map<std::pair<int, int>, unsigned char> myMapR;
	std::map<std::pair<int, int>, unsigned char> myMapG;
	std::map<std::pair<int, int>, unsigned char> myMapB;

	void CountAllPixelsAndUniqeRGB();
	void AddLuminancePositionAndSortArray();
	void UpdatePixelsFromTheOrigin(PPMImage* ImageA, PPMImage* ImageB);
	void SetUpdatedPixelRGBInCorrectPositions();

private:
	int Width = 0;
	int Height = 0;
	std::string Version = "P6";

	void CreateImage();
};

PPMImage::~PPMImage()
{
	DeleteImage();
}

void PPMImage::Save(std::string FileName)
{
	std::ofstream output(FileName, std::ios::binary);

	if (output.is_open())
	{
		output << Version << std::endl;
		output << Width << std::endl;
		output << Height << std::endl;
		output << 255 << std::endl;

		if (Version == "P3")
		{
			for (int i = 0; i < Height; i++)
			{
				for (int j = 0; j < Width; j++)
				{
					output << (int)ImagePtr[i][j].r << ' ';
					output << (int)ImagePtr[i][j].g << ' ';
					output << (int)ImagePtr[i][j].b << '\n';
				}
			}
		}
		else
		{
			for (int i = 0; i < Height; i++)
			{
				for (int j = 0; j < Width; j++)
				{
					output.write((char*) &ImagePtr[i][j], sizeof(RGB)-13);
				}
			}
		}
		output.close();
	}
}

void PPMImage::Read(std::string FileName)
{
	std::ifstream input(FileName, std::ios::binary);

	if (input.is_open())
	{
		int color;
		char ver[3];

		input.read(ver, 3);
		//string type = "";
		//input >> type;
		input >> Width;
		input >> Height;
		input >> color;

		input.read(ver, 1);
		//std::cout << ver[1] << std::endl;
		
		// Create Image
		CreateImage();

		int box;
		if (Version == "P3")
		{
			for (int i = 0; i < Height; i++)
			{
				for (int j = 0; j < Width; j++)
				{
					input >> box;
					ImagePtr[i][j].r = box;

					input >> box;
					ImagePtr[i][j].g = box;

					input >> box;
					ImagePtr[i][j].b = box;
				}
			}
		}
		else
		{
			for (int i = 0; i < Height; i++)
			{
				for (int j = 0; j < Width; j++)
				{
					input.read((char*) &ImagePtr[i][j], sizeof(RGB)-13);
				}
			}
			input.close();
		}
	}
}


void PPMImage::CreateImage()
{
	if (ImagePtr != nullptr)
	{
		DeleteImage();
	}

	ImagePtr = new RGB * [Height];

	for (int i = 0; i < Height; i++)
	{
		ImagePtr[i] = new RGB[Width];

		for (int j = 0; j < Width; j++)
		{
			ImagePtr[i][j].r = 255;
			ImagePtr[i][j].g = 255;
			ImagePtr[i][j].b = 255;
		}
	}

}

void PPMImage::AddLuminancePositionAndSortArray()
{
	const float r = 0.299f;
	const float g = 0.587f;
	const float b = 0.114f;
	float LuminanceValue;

	for (int i = 0; i < Height; i++)
	{
		for (int j = 0; j < Width; j++)
		{
			LuminanceValue = ImagePtr[i][j].r * r + ImagePtr[i][j].g * g + ImagePtr[i][j].b * b;
			ImagePtr[i][j].luminanceValue = LuminanceValue;
			ImagePtr[i][j].x = j;
			ImagePtr[i][j].y = i;
			LumPosPixelsArray.push_back(ImagePtr[i][j]);
		}
		//UpdateBar(i, Height);
	}
	// Sort the Luminance Values
	std::sort(LumPosPixelsArray.begin(), LumPosPixelsArray.end(), [](const RGB& lhs, const RGB& rhs){ return lhs.luminanceValue < rhs.luminanceValue; });
}

void PPMImage::UpdatePixelsFromTheOrigin(PPMImage* A, PPMImage* B)
{
	if (A == nullptr) return;
	if (B == nullptr) return;

	for (int Pixel = 0; Pixel < A->LumPosPixelsArray.size(); ++Pixel)
	{
		// Setting Positions
		A->LumPosPixelsArray[Pixel].x = B->LumPosPixelsArray[Pixel].x;
		A->LumPosPixelsArray[Pixel].y = B->LumPosPixelsArray[Pixel].y;
		// Setting Colors
		myMapR[std::make_pair(A->LumPosPixelsArray[Pixel].x, A->LumPosPixelsArray[Pixel].y)] = A->LumPosPixelsArray[Pixel].r;
		myMapG[std::make_pair(A->LumPosPixelsArray[Pixel].x, A->LumPosPixelsArray[Pixel].y)] = A->LumPosPixelsArray[Pixel].g;
		myMapB[std::make_pair(A->LumPosPixelsArray[Pixel].x, A->LumPosPixelsArray[Pixel].y)] = A->LumPosPixelsArray[Pixel].b;
	}

}
void PPMImage::SetUpdatedPixelRGBInCorrectPositions()
{
	// Set Updated Pixel RGB in Correct Positions
	printf("Setting up Updated RGB Pixels in Correct Positions using the Map:\n");
	for (int i = 0; i < Height; i++)
	{
		for (int j = 0; j < Width; j++)
		{
			ImagePtr[i][j].r = myMapR[std::make_pair(j, i)];
			ImagePtr[i][j].g = myMapG[std::make_pair(j, i)];
			ImagePtr[i][j].b = myMapB[std::make_pair(j, i)];
		}
		UpdateBar(i, Height);
	}
}
void PPMImage::CountAllPixelsAndUniqeRGB()
{
	std::set<std::vector<unsigned char>> SetOfUniqueRGB;
	for (int i = 0; i < Height; i++)
	{
		for (int j = 0; j < Width; j++)
		{
			//PixelsArray.push_back(ImagePtr[i][j]);
			SetOfUniqueRGB.insert({ ImagePtr[i][j].r, ImagePtr[i][j].g, ImagePtr[i][j].b });
		}
		//UpdateBar(i, Height);
	}
	//printf("%s", "\nNumber Of All Pixels in Picture:\n");
	//// Number Of Pixels
	//std::cout << PixelsArray.size() << std::endl;

	size_t const numUniqueElements = SetOfUniqueRGB.size();
	std::cout << numUniqueElements << std::endl;
}

void PPMImage::DeleteImage()
{
	if (ImagePtr != nullptr)
	{
		for (int i = 0; i < Height; i++)
		{
			delete ImagePtr[i];
		}

		delete ImagePtr;
	}
}

void PPMImage::Resize(int Height, int Width)
{
	RGB	**ImageResized = new RGB*[Height];

	for (int i = 0; i < Height; i++)
	{
		ImageResized[i] = new RGB[Width];
		for (int j = 0; j < Width; j++)
		{
			ImageResized[i][j].r = 255;
			ImageResized[i][j].g = 255;
			ImageResized[i][j].b = 255;
		}
	}

	for (int i = 0; i < Height; i++)
	{
		for (int j = 0; j < Width; j++)
		{
			ImageResized[i][j] = ImagePtr[i * this->Height / Height][j * this->Width / Width];
		}
	}
	DeleteImage();
	ImagePtr = ImageResized;

	this->Height = Height;
	this->Width = Width;
}

const int PROG_BAR_LENGTH = 30;

void UpdateBar(int PercentDone, int MaxValue)
{
	const int NumChars = PercentDone * PROG_BAR_LENGTH / MaxValue;
	printf("\r[");
	for (int i = 0; i < NumChars; i++)
	{
		printf("#");
	}
	for (int i = 0; i < PROG_BAR_LENGTH- NumChars; i++)
	{
		printf(" ");
	}
	const int PercentDoneShown = (PercentDone * 100) / MaxValue;
	printf("] %d%% Done", PercentDoneShown);
	fflush(stdout);
	
}

struct Timer
{
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<float> duration;

	Timer()
	{
		start = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;

		float ms = duration.count() * 1000.f;
		std::cout << "Program's execution took: " << ms << "ms" << std::endl;
	}
};
int main()
{
	Timer timer;
	printf("====== IMAGE PAINTER 0.1 ======\n");
	printf("In Solution directory we have Picture A and B.\nProgram creates Picture C using Picture B as a base with picture's A colors\n");
	//========================================================
	// Load JPEG image
	CImg<unsigned char> imA("obrazA.jpg");

	printf("obrazA Loaded\n");
	// Save as PPMImage
	imA.save("A.ppm");

	printf("obrazA Saved as A.ppm\n");

	// Load JPEG image
	CImg<unsigned char> imB("obrazB.jpg");

	printf("obrazB Loaded\n");
	// Save as PPMImage
	imB.save("B.ppm");

	printf("obrazB Saved as B.ppm\n");
	//========================================================

	PPMImage PPMImageA;
	PPMImageA.Read("A.ppm");
	printf("A.ppm Read\n");
	PPMImage PPMImageB;
	PPMImageB.Read("B.ppm");
	printf("B.ppm Read\n");

	//========================================================
	PPMImage* PointerImageA = &PPMImageA;
	PPMImage* PointerImageB = &PPMImageB;

	if (PointerImageA == nullptr || PointerImageB == nullptr) return 0;

	if (!(PointerImageA->GetHeight() == PointerImageB->GetHeight() && PointerImageA->GetWidth() == PointerImageB->GetWidth()))
	{
		printf("Image Sizes are different! Resizing.\n");
	}
	if (PointerImageA->GetHeight() > PointerImageB->GetHeight() && PointerImageA->GetWidth() > PointerImageB->GetWidth())
	{
		// Resize the Image A:
		PPMImageA.Resize(PointerImageB->GetHeight(), PointerImageB->GetWidth());
		PPMImageA.Save("ResultA.ppm");

		printf("ResultA.ppm Resized\n");
	}
	else
	{
		// Resize the Image B:
		PPMImageB.Resize(PointerImageA->GetHeight(), PointerImageA->GetWidth());
		PPMImageB.Save("ResultB.ppm");

		printf("ResultB.ppm Resized\n");
		printf("\n");
	}

	//================================================================================================================
	//								ASYNC LOADING
	//================================================================================================================
	printf("Loading asynchronously:\n");
	printf("Pixels of Picture A and B into Arrays and sorting by luminance each one...\n");
	const auto result1 = std::async(std::launch::async, &PPMImage::AddLuminancePositionAndSortArray, &PPMImageA);
	const auto result2 = std::async(std::launch::async, &PPMImage::AddLuminancePositionAndSortArray, &PPMImageB);
	result1._Get_value();
	result2._Get_value();

	// With or Without Counting Pixels
#if TurnOnOffCountingPixels
	const auto result3 = std::async(std::launch::async, &PPMImage::CountAllPixelsAndUniqeRGB, &PPMImageA);
	const auto result4 = std::async(std::launch::async, &PPMImage::CountAllPixelsAndUniqeRGB, &PPMImageB);
#endif

	printf("Setting up Positions and Colors into the Map...It takes a min or two...\n");
	const auto result5 = std::async(std::launch::async, &PPMImage::UpdatePixelsFromTheOrigin, &PPMImageB, PointerImageA, PointerImageB);
	printf("Counting Unique Colors...\n");
#if TurnOnOffCountingPixels
	printf("\n");
	printf("Number Of All Unique Colors of Picture A:\n");
	result3._Get_value();
	printf("Number Of All Unique Colors of Picture B:\n");
	result4._Get_value();
	printf("\n");
#endif
	result5._Get_value();
	//================================================================================================================

	PPMImageB.SetUpdatedPixelRGBInCorrectPositions();
	printf("\nSaving Results...\n");
	PPMImageA.Save("ResultA.ppm");
	PPMImageB.Save("ResultB.ppm");
	printf("\nResultA.PPMImage and ResultB.ppm Saved\n");
	//========================================================
	// Load PPMImage image
	CImg<unsigned char> PPMImageb("ResultB.ppm");
	// Save as 
	PPMImageb.save("C.png");
	printf("====== C.png Saved ======\n");
	//========================================================
	// Deleting temp files
	if (remove("ResultA.PPM") != 0)
	{
		perror("Error deleting file");
	}
	else
	{
		puts("ResultA.PPM File successfully deleted");
	}
	if (remove("ResultB.PPM") != 0)
	{
		perror("Error deleting file");
	}
	else
	{
		puts("ResultB.PPM File successfully deleted");
	}
	if (remove("A.PPM") != 0)
	{
		perror("Error deleting file");
	}
	else
	{
		puts("A.PPM File successfully deleted");
	}
	if (remove("B.PPM") != 0)
	{
		perror("Error deleting file");
	}
	else
	{
		puts("B.PPMImage File successfully deleted");
	}

	return 0;
}
