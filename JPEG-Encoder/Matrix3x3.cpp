#include "stdafx.h"
#include "Matrix3x3.h"

Matrix3x3::Matrix3x3()
{}

Matrix3x3::Matrix3x3(const float data[3][3])
	: data{ {data[0][1], data[0][2], data[0][3]}, { data[1][1], data[1][2], data[1][3] }, { data[2][1], data[2][2], data[2][3] } }
{}

Matrix3x3::~Matrix3x3()
{}

Pixel Matrix3x3::operator*(const Pixel& pixel)
{
	return Pixel(
			data[0][0] * pixel.getColorValue(0) + data[0][1] * pixel.getColorValue(1) + data[0][1] * pixel.getColorValue(2),
			data[1][0] * pixel.getColorValue(0) + data[1][1] * pixel.getColorValue(1) + data[1][1] * pixel.getColorValue(2),
			data[2][0] * pixel.getColorValue(0) + data[2][1] * pixel.getColorValue(1) + data[2][1] * pixel.getColorValue(2)
		);
}