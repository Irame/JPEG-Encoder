#include "stdafx.h"
#include "ColorChannel.h"

ColorChannel::ColorChannel()
	: width(0), height(0), data(nullptr)
{
}

ColorChannel::ColorChannel(ColorName colorName, int width, int height)
	: colorName(colorName), width(width), height(height)
{
	data = new float*[width];
	for (int i = 0; i < width; i++)
	{
		data[i] = new float[height];
		memset(data[i], 0, height*sizeof(float));
	}
}

ColorChannel::~ColorChannel()
{
	for (int i = 0; i < width; i++)
	{
		delete[] data[i];
	}
	delete[] data;
}

void ColorChannel::setColorValue(float value, int x, int y)
{
	data[x][y] = value;
}

float* ColorChannel::operator[](int idx) const
{
	return data[idx];
}

int ColorChannel::getWidth() const
{
	return width;
}

int ColorChannel::getHeight() const
{
	return height;
}

ColorName ColorChannel::getColorName() const
{
	return colorName;
}

void ColorChannel::scale(float factor)
{
	if (factor == 0) {
		for (int i = 0; i < width; i++)
		{
			delete data[i];
		}
		delete data;

		//set new data
		data = nullptr;
		width = 0;
		height = 0;
		return;
	}

	//calculate new resolution
	int newWidth = width*factor;
	int newHeight = height*factor;

	//aquire new memory for the scaled pixels
	float** newData = new float*[newWidth];
	for (int i = 0; i < newWidth; i++) {
		newData[i] = new float[newHeight];
	}

	//calculate the square that will be combined to one pixel
	int range = (1 / factor)+0.5;
	
	//sampleFactor is used to increment the currently used original pixels
	float sampleFactor = 1 / factor;
	float currentWidth = 0;
	float currentHeigth = 0;

	//foreach new pixel gets the median over all original Pixels that are inside a square as big as range
	for (int i = 0; i < newWidth; i++) {
		for (int j = 0; j < newHeight; j++) {
			float sample = 0.0f;
			int sampleCount = 0;
			for (int k = currentWidth; k < int(currentWidth + range); k++) {
				for (int l = currentHeigth; l < int(currentHeigth + range); l++) {
					sample += data[k][l];
					sampleCount++;
				}
			}
			newData[i][j] = sample / sampleCount;
			currentHeigth += sampleFactor;
		}
		currentHeigth = 0;
		currentWidth += sampleFactor;
	}

	//clean old data
	for (int i = 0; i < width; i++)
	{
		delete data[i];
	}
	delete data;

	//set new data
	data = newData;
	width = newWidth;
	height = newHeight;
}
