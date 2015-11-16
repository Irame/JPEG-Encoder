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

	float* operator[](size_t row)
	{
		assert(row <= 8);
		return data[row];
	}

	~PointerMatrix() {};
};

