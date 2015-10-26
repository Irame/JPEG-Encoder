#include "stdafx.h"
#include "BitBuffer.h"
#include <algorithm>

BitBuffer::BitBuffer(size_t initialBufferSizeInBit) 
	: bufferSize(initialBufferSizeInBit), data(nullptr), bitPos(0)
{
	size_t sizeInByte = bufferSizeInByte();
	data = std::unique_ptr<byte[]>(new byte[sizeInByte]);
	memset(data.get(), 0, sizeInByte);
}

void BitBuffer::pushBits(size_t numOfBits, byte* srcBuffer)
{
	if (bitPos + numOfBits > bufferSize)
		growBuffer();

	byte freeBits = 8 - bitPos % 8; // number of bits to fill data up to byte boundary
	byte byteOffset = bitPos / 8;

	if (numOfBits < freeBits)
	{
		// copy source to dest and reset overlaped bits back to 0
		data[byteOffset] |= (srcBuffer[0] >> (8 - freeBits)) & byte(0xff << (freeBits - numOfBits));
		bitPos += numOfBits;
	}
	else
	{
		// fill current data byte
		data[byteOffset++] |= (srcBuffer[0] >> (8 - freeBits));
		bitPos += freeBits;

		if (numOfBits - freeBits == 0)
			return;

		size_t srcOffset = freeBits;
		size_t srcByteOffset = 0;
		byte leftCount = 8 - freeBits;
		while (srcOffset < numOfBits)
		{
			data[byteOffset++] = joinTwoBytes(srcBuffer[srcByteOffset++], srcBuffer[srcByteOffset], leftCount);
			bitPos += 8;
			srcOffset += 8;
			leftCount = 8 - leftCount;
		}
		data[byteOffset - 1] &= 0xff << (srcOffset - numOfBits);
		bitPos -= (srcOffset - numOfBits);
	}
}

void BitBuffer::growBuffer()
{
	// fixme: reallocate does not copy the values
	int newBufferSize = bufferSizeInByte() * 2;
	void* newPtr = realloc(data.get(), newBufferSize);
	data.reset(static_cast<byte*>(newPtr));
	bufferSize = newBufferSize * 8;
}

size_t BitBuffer::bufferSizeInByte()
{
	return (bufferSize + 7) / 8;
}

byte BitBuffer::joinTwoBytes(byte leftByte, byte rightByte, size_t leftCount)
{
	size_t rightCount = 8 - leftCount;

	leftByte <<= rightCount;
	rightByte >>= leftCount;

	byte leftMask = 0xff << rightCount;
	byte rightMask = 0xff >> leftCount;

	return leftByte & leftMask | rightByte & rightMask;
}