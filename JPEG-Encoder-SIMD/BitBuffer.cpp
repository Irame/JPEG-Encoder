#include "stdafx.h"
#include "BitBuffer.h"
#include <algorithm>
#include <fstream>

BitBuffer::BitBuffer(size_t initialBufferSizeInBit) 
	: bufferSizeInByte((initialBufferSizeInBit + 7)/8), data(bufferSizeInByte, 0), dataBitOffset(0)
{}

inline size_t BitBuffer::getSize() const
{
	return dataBitOffset;
}

inline size_t BitBuffer::getCapacity() const
{
	return bufferSizeInByte * 8;
}

void BitBuffer::pushBit(bool val)
{
	ensureFreeSpace(1);
	if (val)
	{
		auto indices = lldiv(dataBitOffset, 8);
		data[indices.quot] |= 1 << 7 - indices.rem;
	}
	dataBitOffset++;
}

void BitBuffer::pushBits(size_t numOfBits, void* srcBufferVoid, size_t offset)
{
	ensureFreeSpace(numOfBits);

	byte* srcBuffer = static_cast<byte*>(srcBufferVoid);
	auto indices = lldiv(dataBitOffset, 8);
	byte freeBits = 8 - indices.rem; // number of bits to fill data up to byte boundary
	byte byteOffset = indices.quot;

	// needs improvement
	if (offset > 0) {
		srcBuffer += offset / 8;
		offset %= 8;

		size_t bitsToSrcByteBoundary = 8 - offset;
		size_t bitsToWrite = std::min(bitsToSrcByteBoundary, numOfBits);

		if (freeBits < bitsToWrite)
		{
			data[byteOffset++] |= (srcBuffer[0] & (0xff >> offset)) >> (8 - freeBits - offset);
			numOfBits -= freeBits;
			bitsToWrite -= freeBits;
			dataBitOffset += freeBits;
			freeBits = 8;
			if (bitsToWrite > 0)
			{
				data[byteOffset] |= srcBuffer[0] << 8 - bitsToWrite;
				numOfBits -= bitsToWrite;
				dataBitOffset += bitsToWrite;
				freeBits -= bitsToWrite;
			}
		}
		else if (freeBits >= bitsToWrite)
		{
			data[byteOffset] |= (srcBuffer[0] & (0xff >> offset)) << offset >> (8 - freeBits);
			numOfBits -= bitsToWrite;
			dataBitOffset += bitsToWrite;
			indices = lldiv(dataBitOffset, 8);
			freeBits = 8 - indices.rem;
			byteOffset = indices.quot;
			data[byteOffset] &= 0xff << freeBits;
		}
		srcBuffer++;
	}

	if (numOfBits <= freeBits)
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

bool BitBuffer::getBit(size_t index) const
{
	auto indices = lldiv(index, 8);
	return data[indices.quot] & (1 << (7 - indices.rem));
}

void BitBuffer::getBits(size_t index, byte* out, size_t numOfBits) const
{
	if (numOfBits == 0) return;

	auto indices = lldiv(index, 8);
	byte leftCount = 8 - indices.rem;

	size_t bitsProcessed = 0;
	size_t destOffset = 0; 
	size_t byteIndex = indices.quot;
	while (numOfBits > bitsProcessed)
	{
		out[destOffset++] = joinTwoBytes(data[byteIndex++], data[byteIndex], leftCount);
		bitsProcessed += 8;
	}
	out[destOffset - 1] &= 0xff << (bitsProcessed - numOfBits);
}

inline void BitBuffer::ensureFreeSpace(size_t numOfBits)
{
	if (numOfBits + dataBitOffset > getCapacity())
	{
		bufferSizeInByte = (bufferSizeInByte + 1) * 2;
		data.resize(bufferSizeInByte, 0);
	}
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
	fileStream.write(reinterpret_cast<char*>(data.data()), (dataBitOffset + 7) / 8);
	fileStream.flush();
	fileStream.close();
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