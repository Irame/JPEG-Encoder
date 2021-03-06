#pragma once
#include <array>

template<typename T>
using ZigZagReorderdArray = std::array<T, 64>;

typedef ZigZagReorderdArray<size_t> OffsetArray;

// Precalculated for the common 8x8 case
static OffsetArray ZigZagOffsets8x8 {
	0,  1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

// Takes a 64 pointers and returns the same 64 pointers
// reorders by Zig-Zag using a static offset array.
template<typename T>
static ZigZagReorderdArray<T> reorderByZigZag(const T* collection)
{
	ZigZagReorderdArray<T> result;
	for (size_t i = 0; i < result.size(); i++)
	{
		result[i] = collection[ZigZagOffsets8x8[i]];
	}
	return result;
}

// Calculates a Zig-Zag Jumptable with a given image width.
// The resulting OffsetArray can be used to access a pointer
// in ZigZag order. The image width is used to jump across rows.
static OffsetArray createZigZagOffsetArray(size_t imageWidth)
{
	OffsetArray ptrArray;
	size_t curOffset;

	for (size_t i = 0; i < ptrArray.size(); i++)
	{
		curOffset = ZigZagOffsets8x8[i];
		ptrArray[i] = (curOffset % 8) + (imageWidth * (curOffset / 8));
	}

	return ptrArray;
}


// Takes a 64 pointers and returns the same 64 pointers
// reorders by Zig-Zag using loops and stuff.
// Returns the same result like reorderByZigZag() but possibly slower.
template<typename T>
static ZigZagReorderdArray<T> reorderByZigZagSlow(const T* collection)
{
	const size_t width = 8;
	const size_t height = 8;
	const size_t dataSize = width * height;

	int64_t diagonalStep = int64_t(1) - width;

	size_t lastLine = (height - 1) * width;

	ZigZagReorderdArray<T> result;

	size_t curPos = 0;
	size_t index = 0;
	while (index < dataSize)
	{
		result[index++] = (collection[curPos]);
		if (curPos % 2 == 0 && curPos / width == 0  // top border
			|| curPos >= lastLine && curPos % 2 == 0)  // bottom border
		{
			curPos++;
			diagonalStep = -diagonalStep;
		}
		else if (
			(curPos - width) % (2 * width) == 0  // left border
			|| curPos >= width && curPos % width == width - 1 && (curPos / width) % 2 == 1) // right border
		{
			curPos += width;
			diagonalStep = -diagonalStep;
		}
		else
		{
			curPos += diagonalStep;
		}
	}

	return result;
}