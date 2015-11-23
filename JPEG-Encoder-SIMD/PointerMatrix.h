#pragma once

#include <cassert>
#include <array>

class PointerMatrix
{
private:
	float* data[8];
public:
	PointerMatrix(float* r0, float* r1, float* r2, float* r3, float* r4, float* r5, float* r6, float* r7)
		: data{ r0, r1, r2, r3, r4, r5, r6, r7 }
	{ }

	PointerMatrix(float* r)
		: data{ &r[0], &r[8], &r[16], &r[24], &r[32], &r[40], &r[48], &r[56] }
	{ }

	~PointerMatrix() {};

	float* operator[](size_t row) const
	{
		assert(row <= 8);
		return data[row];
	}

	float at(size_t row, size_t col) const
	{
		assert(row <= 7, col <= 7);
		return data[row][col];
	}

	float atTransposed(size_t row, size_t col) const
	{
		assert(row <= 7, col <= 7);
		return data[col][row];
	}
};


class mat8x8
{
private:
	std::array<float, 64> data;

public:
	mat8x8() : data({}) {}; // init with zeros

	mat8x8(float* matrix)
	{
		memcpy_s(data.data(), 64, matrix, 64);
	}

	mat8x8(float* r0, float* r1, float* r2, float* r3, float* r4, float* r5, float* r6, float* r7)
	{ 
		memcpy_s(&data[0],  8*sizeof(float), r0, 8*sizeof(float));
		memcpy_s(&data[8],  8*sizeof(float), r1, 8*sizeof(float));
		memcpy_s(&data[16], 8*sizeof(float), r2, 8*sizeof(float));
		memcpy_s(&data[24], 8*sizeof(float), r3, 8*sizeof(float));
		memcpy_s(&data[32], 8*sizeof(float), r4, 8*sizeof(float));
		memcpy_s(&data[40], 8*sizeof(float), r5, 8*sizeof(float));
		memcpy_s(&data[48], 8*sizeof(float), r6, 8*sizeof(float));
		memcpy_s(&data[56], 8*sizeof(float), r7, 8*sizeof(float));
	}


	~mat8x8() {};

	float& operator[](size_t idx) { return data[idx]; }
	const float& operator[](size_t idx) const { return data[idx]; }

	const float& at(size_t row, size_t col) const
	{
		return data[row * 8 + col];
	}

	float& at(size_t row, size_t col)
	{
		return data[row * 8 + col];
	}

	const float& atT(size_t row, size_t col) const { return at(col, row); }
	float& atT(size_t row, size_t col) { return at(col, row); }
};
