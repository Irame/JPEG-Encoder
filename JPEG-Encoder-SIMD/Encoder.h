#pragma once
#include "Image.h"
#include "ZigZag.h"
#include "HuffmanCodingByte.h"

class Encoder
{
public:
	enum CoefficientType
	{
		DC = 0, AC = 1
	};

private:
	ImagePtr image;
	
	// first index is ColorChannelName second index is CoefficientType
	std::vector<BEushort> bitPatternDC[3];
	std::vector<byte> categoriesDC[3];
	std::vector<std::vector<BEushort>> bitPatternAC[3];
	std::vector<std::vector<byte>> categoriesAC[3];
	HuffmanTablePtr<byte> huffmanTables[2][2];

	const QTableSet qTables;

	std::vector<PointerMatrix> createBlocks(const ColorChannelName channelName);
	void calculateACValues(const std::vector<PointerMatrix>& blocks, const OffsetArray& zigZag, const ColorChannelName channelName);
	void calculateDCValues(const std::vector<PointerMatrix>& blocks, const OffsetArray& zigZag, const ColorChannelName channelName);

	// Optimized bit category lookup of an AC/DC value
	byte lookupBitCategory(short value) const;

	HuffmanTablePtr<byte> createHuffmanTable(const CoefficientType type, const ColorChannelName channelName);

	void pushBlock(BitBuffer &bitBuffer, ColorChannelName channelName, size_t block);

	void reduceWidthResolutionColorChannel(ColorChannelName channelIdx, int factor, ReductionMethod method);
	void reduceHeightResolutionColorChannel(ColorChannelName channelIdx, int factor, ReductionMethod method);

public:
	Encoder(const Image& image, const QTableSet& qtables);
	Encoder(ImagePtr imagePtr, const QTableSet& qtables);

	void convertToYCbCr();
	void convertToRGB();

	void multiplyColorChannelBy(ColorChannelName colorChannel, float val);

	void reduceResolutionBySchema();

	HuffmanTablePtr<byte> getHuffmanTable(CoefficientType type, ColorChannelName channelName);
	void serialize(BitBuffer &bitBuffer);
	void serializeScanData(BitBuffer& bitBuffer);

	void applyDCT(ColorChannelName colorChannelName);
};

typedef std::shared_ptr<Encoder> EncoderPtr;

