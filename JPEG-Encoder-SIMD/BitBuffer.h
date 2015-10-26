#pragma once
#include <memory>
#include <iostream>

typedef unsigned char byte;

class BitBuffer
{
	size_t bufferSize;
	std::unique_ptr<byte[]> data;

	size_t dataBitOffset;		// position in 'data' in bits

public:
	BitBuffer(size_t initialBufferSize);
	
	void pushBits(size_t numOfBits, byte* buffer);
	void growBuffer();
	size_t bufferSizeInByte();
	byte joinTwoBytes(byte leftByte, byte rightByte, size_t leftCount);
	
	void writeToFile(std::string file);

	friend std::ostream& operator<<(std::ostream& strm, const BitBuffer& bitBuffer);
};