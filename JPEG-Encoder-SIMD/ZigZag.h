#pragma once
#include <array>


template<typename T>
using ZigZagReorderdArray = std::array<T, 64>;

template<typename T>
static ZigZagReorderdArray<T> reorderByZigZag(const T* collection)
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


typedef std::array<size_t, 64> OffsetArray;
static OffsetArray createZigZagOffsetArray(size_t width)
{
	OffsetArray ptrArray;
	for (size_t i = 0; i < 64; i++)
	{
		ptrArray[i] = (i % 8) + (width * (i / 8));
	}
	return reorderByZigZag(ptrArray.data());
}