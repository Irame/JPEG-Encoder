#pragma once
#include "Pixel.h"

class Matrix3x3
{
	float data[3][3];

public:
	Matrix3x3();
	~Matrix3x3();

	Pixel operator*(const Pixel& pixel);
};

