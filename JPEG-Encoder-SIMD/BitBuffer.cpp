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

size_t BitBuffer::getSize() const
{
	return dataBitOffset;
}

size_t BitBuffer::getCapacity() const
{
	return bufferSize;
}

void BitBuffer::pushBit(bool val)
{
	if (val)
	{
		size_t dataByteOffset = dataBitOffset / 8;
		byte curByteBitOffset = dataBitOffset % 8;
		data[dataByteOffset] |= 1 << 7 - curByteBitOffset;
	}
	dataBitOffset++;
}

void BitBuffer::pushBits(size_t numOfBits, byte* srcBuffer, size_t offset)
{
	if (dataBitOffset + numOfBits > bufferSize)
		growBuffer();

	byte freeBits = 8 - dataBitOffset % 8; // number of bits to fill data up to byte boundary
	byte byteOffset = dataBitOffset / 8;

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
			freeBits = 8 - dataBitOffset % 8;
			byteOffset = dataBitOffset / 8;
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