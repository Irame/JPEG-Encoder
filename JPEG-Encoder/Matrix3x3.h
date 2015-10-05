#pragma once
#include "Vector.h"

class Matrix3x3
{
	float data[3][3];

public:
	Matrix3x3();
	Matrix3x3(const float data[3][3]);
	~Matrix3x3();

	Vector operator*(const Vector& v);
};

