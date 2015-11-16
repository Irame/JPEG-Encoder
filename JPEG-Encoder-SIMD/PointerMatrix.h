#pragma once

#include <cassert>

class PointerMatrix
{
private:
	float* data[8];
public:
	PointerMatrix(float* r0, float* r1, float* r2, float* r3, float* r4, float* r5, float* r6, float* r7)
		: data{ r0, r1, r2, r3, r4, r5, r6, r7 }
	{ }

	~PointerMatrix() {};

	float* operator[](size_t row) const
	{
		assert(row <= 8);
		return data[row];
	}

	float at(size_t row, size_t col) const
	{
		assert(x <= 7, y <= 7);
		return data[row][col];
	}

	float atTransposed(size_t row, size_t col) const
	{
		assert(x <= 7, y <= 7);
		return data[col][row];
	}
};

