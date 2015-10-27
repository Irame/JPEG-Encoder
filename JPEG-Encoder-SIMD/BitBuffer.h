#pragma once
#include <memory>
#include <iostream>

typedef unsigned char byte;

class BitBuffer
{
	size_t bufferSize;
	std::unique_ptr<byte[]> data;

	size_t dataBitOffset;		// position in 'data' in bits

	void growBuffer();
	byte joinTwoBytes(byte leftByte, byte rightByte, size_t leftCount);

public:
	BitBuffer(size_t initialBufferSize);
	
	void pushBit(bool val);
	void pushBits(size_t numOfBits, byte* buffer, size_t offset = 0);

	size_t bufferSizeInByte();
	
	void writeToFile(std::string file);

	friend std::ostream& operator<<(std::ostream& strm, const BitBuffer& bitBuffer);
};