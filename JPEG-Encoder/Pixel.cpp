#include "stdafx.h"
#include "Pixel.h"
#include "Matrix3x3.h"

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

template<>
void Pixel::switchColorCoding<RGB, RGB>()
{}

template<>
void Pixel::switchColorCoding<RGB, YCbC>()
{
	static float transformMatrixArray[3][3] {
		{0.299f, 0.587f, 0.114f},
		{-0.1687f, -0.3312f, 0.5f},
		{0.5f, -0.4186f, -0.08}
	};
	static Matrix3x3 transformMatrix(transformMatrixArray);
	static Pixel transformVector(0.0f, 0.5f, 0.5f);
	*this = transformMatrix * *this ;
	*this += transformVector;
}

template<>
void Pixel::switchColorCoding<YCbC, YCbC>()
{}

template<>
void Pixel::switchColorCoding<YCbC, RGB>()
{
	//TODO: implementation
}