#include "stdafx.h"
#include "BitBuffer.h"
#include <algorithm>
#include <fstream>

BitBuffer::BitBuffer(size_t initialBufferSizeInBit) 
	: bufferSize(initialBufferSizeInBit), data(nullptr), dataBitOffset(0)
{
	size_t sizeInByte = bufferSizeInByte();
	data = std::unique_ptr<byte[]>(new byte[sizeInByte]);
	memset(data.get(), 0, sizeInByte);
}

void BitBuffer::pushBits(size_t numOfBits, byte* srcBuffer)
{
	if (dataBitOffset + numOfBits > bufferSize)
		growBuffer();

	byte freeBits = 8 - dataBitOffset % 8; // number of bits to fill data up to byte boundary
	byte byteOffset = dataBitOffset / 8;

	if (numOfBits < freeBits)
	{
		// copy source to dest and reset overlaped bits back to 0
		data[byteOffset] |= (srcBuffer[0] >> (8 - freeBits)) & byte(0xff << (freeBits - numOfBits));
		dataBitOffset += numOfBits;
	}
	else
	{
		// fill current data byte
		data[byteOffset++] |= (srcBuffer[0] >> (8 - freeBits));
		dataBitOffset += freeBits;

		if (numOfBits - freeBits == 0)
			return;

		size_t srcOffset = freeBits;
		size_t srcByteOffset = 0;
		byte leftCount = 8 - freeBits;
		while (srcOffset < numOfBits)
		{
			data[byteOffset++] = joinTwoBytes(srcBuffer[srcByteOffset++], srcBuffer[srcByteOffset], leftCount);
			dataBitOffset += 8;
			srcOffset += 8;
		}
		data[byteOffset - 1] &= 0xff << (srcOffset - numOfBits);
		dataBitOffset -= (srcOffset - numOfBits);
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

void BitBuffer::writeToFile(std::string filePath)
{
	std::ofstream fileStream;

	fileStream.open(filePath, std::ios::out | std::ios::trunc | std::ios::binary);
	fileStream.write(reinterpret_cast<char*>(data.get()), (dataBitOffset + 7) / 8);
}

std::ostream& operator<<(std::ostream& strm, const BitBuffer& bitBuffer)
{
	size_t bytes = (bitBuffer.dataBitOffset + 7) / 8;
	size_t byteCounter = 0;
	for (int byteOffset = 0; byteOffset < bytes; byteOffset++)
	{
		byte curByte = bitBuffer.data[byteOffset];
		for (int bitMask = 0b10000000; bitMask != 0; bitMask >>= 1)
		{
			if (byteCounter == bitBuffer.dataBitOffset) break;
			strm << (curByte & bitMask ? "1" : "0");
			byteCounter++;
			if (bitMask == 0b10000) strm << " ";
		}
		strm << "  ";
	}

	return strm;
}