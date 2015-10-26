#pragma once
#include <memory>

typedef unsigned char byte;

class BitBuffer
{
	size_t bufferSize;
	std::unique_ptr<byte[]> data;

	size_t bitPos;		// position in 'data' in bits

public:
	BitBuffer(size_t initialBufferSize);
	
	void pushBits(size_t numOfBits, byte* buffer);
	void growBuffer();
	size_t bufferSizeInByte();
	byte joinTwoBytes(byte leftByte, byte rightByte, size_t leftCount);
};

