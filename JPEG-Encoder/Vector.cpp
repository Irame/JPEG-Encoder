#include "stdafx.h"
#include "Vector.h"

Vector::Vector()
	: data{ 0.0f, 0.0f, 0.0f }
{}

Vector::Vector(float v1, float v2, float v3)
	: data{ v1, v2, v3}
{}

Vector::~Vector()
{}

Vector& Vector::operator+=(const Vector& vec)
{
	data[0] += vec.data[0];
	data[1] += vec.data[1];
	data[2] += vec.data[2];
	return *this;
}

const float* Vector::getData() const
{
	return data;
}