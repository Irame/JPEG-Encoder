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
		data[i] = new float[height] {0.0f};
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