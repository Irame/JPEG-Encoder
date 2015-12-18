#pragma once
#include <vector>
#include <memory>

class BitBuffer
{
	size_t bufferSizeInByte;
	std::vector<byte> data;

	// position in 'data' in bits
	size_t dataBitOffset;

	void ensureFreeSpace(size_t numOfBits);
	
	static byte joinTwoBytes(byte leftByte, byte rightByte, size_t leftCount);

	void pushBits(const BitBuffer& buffer, bool escape);
	void pushBits(size_t numOfBits, const void* buffer, bool escape);
	void pushBits(size_t numOfBits, const void* buffer, size_t offset, bool escape);

public:
	BitBuffer(size_t initialBufferSize = 0);
	
	// Get the size in bits (number of set bits)
	size_t getSize() const { return dataBitOffset; };
	// Get the maximum number of bits that can be stored at the moment
	size_t getCapacity() const { return bufferSizeInByte * 8; };

	void fillToByteBorder();

	// Appends a single bit at the end of the BitBuffer
	void pushBit(bool val);

	// Adds the content of buffer.
	void pushBits(const BitBuffer& buffer);
	// Adds a certain number of bits from a specific location with an offset
	void pushBits(size_t numOfBits, const void* buffer, size_t offset);
	// Adds a certain number of bits from a specific location
	void pushBits(size_t numOfBits, const void* buffer);
	

	// Adds the content of buffer. 0xFF bytes will be escaped.
	void pushBitsEscaped(const BitBuffer& buffer);
	// Adds a certain number of bits from a specific location. 0xFF bytes will be escaped.
	void pushBitsEscaped(size_t numOfBits, const void* buffer);
	// Adds a certain number of bits from a specific location with an offset. 0xFF bytes will be escaped.
	void pushBitsEscaped(size_t numOfBits, const void* buffer, size_t offset);
	

	// Returns the bit at the given index
	bool getBit(size_t index) const;
	// Writes the requested bits into an out byte array
	void getBits(size_t index, void* out, size_t numOfBits) const;
	
	// clears conent
	void clear();

	// Writes all bits from the buffer into a file (in binary)
	void writeToFile(std::string file);

	// compares two BitBuffer
	bool operator==(const BitBuffer& other) const;
	bool operator<(const BitBuffer& other) const;
	bool operator>(const BitBuffer& other) const;

	int compare(const BitBuffer& other) const;

	// Converts the Buffer into a string of '0' and '1' in blocks seperated by ' ' or '  '
	friend std::ostream& operator<<(std::ostream& strm, const BitBuffer& bitBuffer);

	// Generic write method
	template<typename T>
	void push(T& value, size_t offset)
	{
		pushBits(sizeof(T) * 8, &value, offset);
	}

	template<typename T>
	void push(T& value)
	{
		pushBits(sizeof(T) * 8, &value);
	}
};

// specialized version for bool
template<>
inline void BitBuffer::push(bool& value, size_t offset)
{
	pushBit(value);
}

typedef std::shared_ptr<BitBuffer> BitBufferPtr;
typedef std::shared_ptr<const BitBuffer> ConstBitBufferPtr;