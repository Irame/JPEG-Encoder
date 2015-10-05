#include "stdafx.h"
#include "Image.h"
#include <fstream>
#include <sstream>
using namespace std;

Image::Image()
	: stepX(1), stepY(1), width(0), heigth(0)
{
	for (int i = 0; i < 3; i++)
		channel[i] = ColorChannel(i, width, heigth);
}

Image::Image(int width, int height)
	: stepX(1), stepY(1), width(width), heigth(heigth)
{
	for (int i = 0; i < 3; i++)
		channel[i] = ColorChannel(i, width, heigth);
}

Image::~Image()
{
}

const ColorChannel& Image::getColorChannel(ColorName colorName) const
{
	return channel[colorName.rgbColorName];
}

ColorCoding Image::getColorCoding()
{
	return colorCoding;
}

void Image::setStep(unsigned int stepX, unsigned int stepY)
{
	this->stepX = stepX;
	this->stepY = stepY;
}

ImagePtr Image::readPPM(std::string path)
{
	ImagePtr resultImage = nullptr;
	enum State {
		None, Size, Pixels
	};
	State state = State::None;
	std::ifstream fileStream = std::ifstream(path);
	string line;
	float maxValue = 1.0f;
	int width = 0;
	int height = 0;
	int currentWidth = 0;
	int currentHeigth = 0;
	int j = 0;

	while (getline(fileStream, line)) {
		if (line == "P3")
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
			resultImage = make_shared<Image>(width, height);

			state = Size;
		} else if (state == State::Size) {
			maxValue = stoi(line);
			state = State::Pixels;
		} else if (state == State::Pixels) {
			std::stringstream stringStream(line);
			string color;
			while (getline(stringStream, color, ' ')) {
				if (color.length() == 0)
					continue;

				resultImage->channel[j].setColorValue(stoi(color) / maxValue, currentWidth, currentHeigth);
				
				j++;
				if (j > 2) {
					j = 0;
					currentWidth++;
					if (currentWidth >= width) {
						currentWidth = 0;
						currentHeigth++;
					}
				}
			}
		}
	}
	return resultImage;
}
