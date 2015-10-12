#include "stdafx.h"
#include "ImageLoader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include "SIMD.h"
#include "lodepng.h"

using namespace std;

ImageLoader::ImageLoader()
{
}

ImageLoader::~ImageLoader()
{
}

ImagePtr ImageLoader::Load(const std::string& filename)
{
	std::string ext = fileExtension(filename);

	if (ext == "png") {
		return LoadPNG(filename);
	} else if (ext == "ppm") {
		return LoadPPM(filename);
	} else {
		std::cout << "Failed to load image. Unknown file extension " << ext << std::endl;
		return nullptr;
	}
}

void ImageLoader::Save(const std::string& filename, ImagePtr image)
{
	std::string ext = fileExtension(filename);

	if (ext == "png") {
		return SavePNG(filename, image);
	} else if (ext == "ppm") {
		return SavePPM(filename, image);
	} else {
		std::cout << "Failed to save image. Unknown file extension " << ext << std::endl;
	}
}

inline int ImageLoader::clamp(int lower, int x, int upper)
{
	return max(lower, min(x, upper));
}

inline std::string ImageLoader::fileExtension(const std::string& filename)
{
	return filename.substr(filename.find_last_of(".") + 1);
}


ImagePtr ImageLoader::LoadPPM(std::string path)
{
	enum State {
		None, Size, Pixels
	};

	State state = State::None;
	ifstream fileStream = ifstream(path);
	string line;
	float maxValue = 1.0f;
	int width = 0;
	int height = 0;
	int currentWidth = 0;
	int currentHeight = 0;
	int j = 0;
	vector<PixelData32> data;

	while (getline(fileStream, line)) {
		if (line == "P3")
			continue;
		if (line.empty())
			continue;
		if (line.front() == '#')
			continue;
		if (state == State::None) {
			std::stringstream stringStream(line);
			string size;
			for (int i = 0; getline(stringStream, size, ' '); i++) {
				if (i == 0)
					width = stoi(size);
				else {
					height = stoi(size);
					//break;
				}
			}
			data.resize(width*height);

			state = Size;
		}
		else if (state == State::Size) {
			maxValue = static_cast<float>(stoi(line));
			state = State::Pixels;
		}
		else if (state == State::Pixels) {
			std::stringstream stringStream(line);
			string color;
			while (getline(stringStream, color, ' ')) {
				if (color.length() == 0)
					continue;

				switch (j)
				{
				case 0: data[currentHeight * width + currentWidth].R = stoi(color) / maxValue; break;
				case 1: data[currentHeight * width + currentWidth].G = stoi(color) / maxValue; break;
				case 2: data[currentHeight * width + currentWidth].B = stoi(color) / maxValue; break;
				default: break;
				}

				j++;
				if (j > 2) {
					j = 0;
					currentWidth++;
					if (currentWidth >= width) {
						currentWidth = 0;
						currentHeight++;
					}
				}
			}
		}
	}

	ImagePtr resultImage = make_shared<Image>(width, height, 16, 16);
	resultImage->setRawPixelData((float*)&data[0]);
	return resultImage;
}

void ImageLoader::SavePPM(std::string path, ImagePtr image)
{
	ofstream fileStream = ofstream(path);

	const size_t width = image->getWidth();
	const size_t height = image->getHeight();
	const int maxVal = 255;

	fileStream << "P3" << endl;
	fileStream << width << " " << height << endl;
	fileStream << maxVal << endl;
	
	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < width; x++)
		{
			PixelData32& p = image->GetPixel(x, y);
			const int r = clamp(0, static_cast<int>(p.R * maxVal), maxVal);
			const int g = clamp(0, static_cast<int>(p.G * maxVal), maxVal);
			const int b = clamp(0, static_cast<int>(p.B * maxVal), maxVal);

			fileStream << r << " " << g << " " << b << "  ";
		}
		fileStream << endl;
	}
}

ImagePtr ImageLoader::LoadPNG(std::string path)
{
	std::vector<unsigned char> imgData;
	unsigned imgWidth, imgHeight;

	unsigned error = lodepng::decode(imgData, imgWidth, imgHeight, path);
	if (error) {
		std::cout << "Failed to decode png " << path << " with error: " << error << ": " << lodepng_error_text(error) << std::endl;
		return nullptr;
	}

	std::vector<float> imgDataFloat(imgData.size());
	for (size_t i = 0; i < imgDataFloat.size(); i++) {
		imgDataFloat[i] = imgData[i] / 255.0f;
	}

	ImagePtr resultImage = make_shared<Image>(imgWidth, imgHeight, 14, 14);
	resultImage->setRawPixelData((float*)&imgDataFloat[0]);
	return resultImage;
}

void ImageLoader::SavePNG(std::string path, ImagePtr image)
{
	const unsigned imgWidth = image->getSimulatedWidth();
	const unsigned imgHeight = image->getSimulatedHeight();

	std::vector<float> data(imgWidth*imgHeight*4);
	image->getRawPixelData(&data[0]);

	std::vector<unsigned char> imgData(data.size());
	for (size_t i = 0; i < imgData.size(); i++) {
		imgData[i] = clamp(0, static_cast<int>(data[i] * 255), 255);
	}

	unsigned error = lodepng::encode(path, imgData, imgWidth, imgHeight);
	if (error) {
		std::cout << "Failed to encode png " << path << " with error: " << error << ": " << lodepng_error_text(error) << std::endl;
	}
}
