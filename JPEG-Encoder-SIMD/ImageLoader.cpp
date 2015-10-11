#include "stdafx.h"
#include "ImageLoader.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "SIMD.h"

using namespace std;

ImageLoader::ImageLoader()
{
}

ImageLoader::~ImageLoader()
{
}

ImagePtr ImageLoader::FromPPM(std::string path)
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
			maxValue = stoi(line);
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

	ImagePtr resultImage = make_shared<Image>(width, height);
	resultImage->setRawPixelData((float*)&data[0]);
	return resultImage;
}

void ImageLoader::SaveToPPM(std::string path, ImagePtr image)
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

inline int ImageLoader::clamp(int lower, int x, int upper)
{
	return max(lower, min(x, upper));
}
