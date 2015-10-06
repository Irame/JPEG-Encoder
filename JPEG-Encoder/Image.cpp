#include "stdafx.h"
#include "Image.h"
#include <fstream>
#include <sstream>
#include "Matrix3x3.h"
using namespace std;

Image::Image()
	: stepX(1), stepY(1), width(0), height(0)
{
	for (int i = 0; i < 3; i++)
		channel[i] = new ColorChannel(i, width, height);
}

Image::Image(int width, int height, ColorCoding coding)
	: stepX(1), stepY(1), width(width), height(height), colorCoding(coding)
{
	for (int i = 0; i < 3; i++)
		channel[i] = new ColorChannel(i, width, height);
}

Image::~Image()
{
}

const ColorChannel& Image::getColorChannel(ColorName colorName) const
{
	return *channel[colorName.rgbColorName];
}

ColorCoding Image::getColorCoding() const
{
	return colorCoding;
}

void Image::setStep(unsigned int stepX, unsigned int stepY)
{
	this->stepX = stepX;
	this->stepY = stepY;
}

void Image::switchColorCoding(ColorCoding newCoding)
{
	if (colorCoding == newCoding) return;
	if (colorCoding == RGB)
	{
		if (newCoding == YCbCr)
		{
			// RGB => YCbCr
			static float transformMatrixArray[3][3]{
				{ 0.299f, 0.587f, 0.114f },
				{ -0.1687f, -0.3312f, 0.5f },
				{ 0.5f, -0.4186f, -0.08 }
			};
			static Matrix3x3 transformMatrix(transformMatrixArray);
			static Vector transformVector(0.0f, 0.5f, 0.5f);
			for (int x = 0; x < width; x++) {
				for (int y = 0; y < height; y++) {
					Vector color(
							(*channel[R])[x][y],
							(*channel[G])[x][y],
							(*channel[B])[x][y]
						);
					Vector resultColor = transformMatrix * color;
					resultColor += transformVector;
					(*channel[R])[x][y] = resultColor.getData()[Y];
					(*channel[G])[x][y] = resultColor.getData()[Cb];
					(*channel[B])[x][y] = resultColor.getData()[Cr];
				}
			}
			colorCoding = newCoding;
		}
	}
	else if (colorCoding == YCbCr)
	{
		if (newCoding == RGB)
		{
			//TODO: Implementation of YCbCr => RGB
		}
	}
}

void Image::scaleColor(ColorName colorName, float factor)
{
	(*channel[colorName.rgbColorName]).scale(factor);
}

ImagePtr Image::readPPM(std::string path)
{
	ImagePtr resultImage = nullptr;
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
			resultImage = make_shared<Image>(width, height, RGB);

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

				resultImage->channel[j]->setColorValue(stoi(color) / maxValue, currentWidth, currentHeigth);
				
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
