#pragma once
#include "Image.h"
#include "ZigZag.h"

class Encoder : public Image
{
public:
	enum CoefficientType
	{
		DC = 0, AC = 1
	};

private:

	//first index is ColorChannelName second index is CoefficientType
	std::vector<BEushort> bitPatternDC[3];
	std::vector<byte> categoriesDC[3];
	std::vector<std::vector<BEushort>> bitPatternAC[3];
	std::vector<std::vector<byte>> categoriesAC[3];
	HuffmanTablePtr<byte> huffmanTables[2][2];
	std::vector<PointerMatrix> blocks[3];

	void ensurePointerMatrix(const ColorChannelName colorChannelName);
	void calculateACValues(const OffsetArray& zigZag, const ColorChannelName colorChannelName);
	void calculateDCValues(const OffsetArray& zigZag, const ColorChannelName colorChannelName);
	HuffmanTablePtr<byte> createHuffmanTable(const CoefficientType type, const ColorChannelName channel);

	void pushBlock(BitBuffer &bitBuffer, ColorChannelName colorChannelName, size_t block);

	void reduceWidthResolutionColorChannel(ColorChannelName channelIdx, int factor, ReductionMethod method);
	void reduceHeightResolutionColorChannel(ColorChannelName channelIdx, int factor, ReductionMethod method);

public:
	Encoder(Image image);

	void convertToYCbCr();
	void convertToRGB();

	void multiplyColorChannelBy(ColorChannelName colorChannel, float val);

	void reduceResolutionBySchema();

	//std::vector<BEushort> getBitPattern(CoefficientType type, ColorChannelName colorChannelName) { return bitPattern[colorChannelName][type]; }
	//std::vector<byte> getCategories(CoefficientType type, ColorChannelName colorChannelName) { return categories[colorChannelName][type]; }
	HuffmanTablePtr<byte> getHuffmanTable(CoefficientType type, ColorChannelName colorChannelName);
	void serialize(BitBuffer &bitBuffer);

	void applyDCT(ColorChannelName colorChannelName);	
};

typedef std::shared_ptr<Encoder> EncoderPtr;

