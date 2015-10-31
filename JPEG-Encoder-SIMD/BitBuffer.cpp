#include "stdafx.h"
#include "BitBuffer.h"
#include <algorithm>
#include <fstream>

BitBuffer::BitBuffer(size_t initialBufferSizeInBit) 
	: bufferSizeInByte(initialBufferSizeInBit == 0 ? 1 : (initialBufferSizeInBit + 7)/8), 
	data(bufferSizeInByte, 0), dataBitOffset(0)
{}

void BitBuffer::pushBit(bool val)
{
	ensureFreeSpace(1);
	if (val)
	{
		size_t dataByteOffset = dataBitOffset / 8;
		byte curByteBitOffset = dataBitOffset % 8;
		data[dataByteOffset] |= 1 << (7 - curByteBitOffset);
	}
	dataBitOffset++;
}

void BitBuffer::pushBits(size_t numOfBits, void* srcBufferVoid, size_t offset)
{
	ensureFreeSpace(numOfBits);

	byte* srcBuffer = static_cast<byte*>(srcBufferVoid);
	size_t freeBits = 8 - dataBitOffset % 8;						// number of bits to fill data up to byte boundary
	size_t byteOffset = dataBitOffset / 8;

	if (offset >= 8)
	{
		// skip whole bytes in offset
		srcBuffer += offset / 8;
		offset %= 8;
	}

	// needs improvement
	if (offset > 0) {
		size_t bitsToSrcByteBoundary = 8 - offset;
		size_t bitsToWrite = std::min(bitsToSrcByteBoundary, numOfBits);		// has range 1..8

		if (freeBits < bitsToWrite) // not enough space in actual data byte
		{
			// fill the current data byte
			data[byteOffset++] |= (srcBuffer[0] & (0xff >> offset)) >> (8 - freeBits - offset);
			numOfBits -= freeBits;
			bitsToWrite -= freeBits;
			dataBitOffset += freeBits;
			freeBits = 8;

				// write remaining bitsToWrite to the next data byte
				data[byteOffset] |= srcBuffer[0] << (8 - bitsToWrite);
				numOfBits -= bitsToWrite;
				dataBitOffset += bitsToWrite;
				freeBits -= bitsToWrite;
			}
		else if (freeBits >= bitsToWrite)
		{
			// write all bitsToWrite to the current data byte
			data[byteOffset] |= (srcBuffer[0] & (0xff >> offset)) << offset >> (8 - freeBits);
			data[byteOffset] &= 0xff << (freeBits - bitsToWrite);
			numOfBits -= bitsToWrite;
			dataBitOffset += bitsToWrite;
			freeBits = 8 - dataBitOffset % 8;
			byteOffset = dataBitOffset / 8;
		}
		srcBuffer++;
	}

	if (numOfBits == 0) return;

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

		size_t srcOffset = freeBits;
		size_t srcByteOffset = 0;
		size_t leftCount = 8 - freeBits;
		// go through the remaining number of bits byte by byte
		while (srcOffset < numOfBits)
		{
			// fill data with joined whole bytes
			data[byteOffset++] = joinTwoBytes(srcBuffer[srcByteOffset], srcBuffer[srcByteOffset + 1], leftCount);
			srcByteOffset++;
			dataBitOffset += 8;
			srcOffset += 8;
		}
		// reset overlaped bits back to 0 and adjusting offset
		data[byteOffset - 1] &= 0xff << (srcOffset - numOfBits);
		dataBitOffset -= (srcOffset - numOfBits);
	}
}

bool BitBuffer::getBit(size_t index) const
{
	size_t dataByteOffset = index / 8;
	byte curByteBitOffset = index % 8;
	return 0 != (data[dataByteOffset] & (1 << (7 - curByteBitOffset)));
}

void BitBuffer::getBits(size_t index, byte* out, size_t numOfBits) const
{
	if (numOfBits == 0) return;

	byte leftCount = 8 - index % 8;

	size_t bitsProcessed = 0;
	size_t destOffset = 0; 
	size_t byteIndex = index / 8;
	while (numOfBits > bitsProcessed)
	{
		out[destOffset++] = joinTwoBytes(data[byteIndex], data[byteIndex + 1], leftCount);
		byteIndex++;
		bitsProcessed += 8;
	}
	out[destOffset - 1] &= 0xff << (bitsProcessed - numOfBits);
}

// ensures that there is enought space for numOfBits in the buffer 
inline void BitBuffer::ensureFreeSpace(size_t numOfBits)
{
	size_t neededSpace = (numOfBits + dataBitOffset + 7)/8;
	while (neededSpace > bufferSizeInByte)
	{
		// ctor ensures that bufferSizeInByte > 0
		bufferSizeInByte *= 2;
		data.resize(bufferSizeInByte, 0);
	}
}

// joins leftCount bits from the right side of the leftByte and 8-leftCount bits from the left side of the rightByte to one byte 
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

// converts to format "([01]{4} [01]{4}  )*" -> helpful for testing
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