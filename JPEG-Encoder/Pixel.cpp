#include "stdafx.h"
#include "Pixel.h"


Pixel::Pixel() :
	data{ 0.0f, 0.0f, 0.0f }
{}


Pixel::Pixel(float v1, float v2, float v3) :
	data{ v1, v2, v3 }
{}

Pixel::~Pixel()
{}

Pixel& Pixel::operator+=(const Pixel& pixel)
{
	data[0] += pixel.getColorValue(0);
	data[1] += pixel.getColorValue(1);
	data[2] += pixel.getColorValue(2);
	return *this;
}