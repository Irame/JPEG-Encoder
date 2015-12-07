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
	std::vector<BEushort> bitPattern[3][2];
	std::vector<byte> categories[3][2];
	HuffmanTablePtr<byte> huffmanTables[2][2];
	std::vector<PointerMatrix> blocks[3];

	void ensurePointerMatrix(const ColorChannelName colorChannelName);
	void calculateACValues(const OffsetArray& zigZag, const ColorChannelName colorChannelName);
	void calculateDCValues(const OffsetArray& zigZag, const ColorChannelName colorChannelName);
	void createHuffmanTable(const CoefficientType type, const ColorChannelName colorChannelName);

	void reduceWidthResolutionColorChannel(ColorChannelName channelIdx, int factor, ReductionMethod method);
	void reduceHeightResolutionColorChannel(ColorChannelName channelIdx, int factor, ReductionMethod method);

public:
	Encoder(Image image);

	void convertToYCbCr();
	void convertToRGB();

	void multiplyColorChannelBy(ColorChannelName colorChannel, float val);

	void reduceResolutionBySchema();

	HuffmanTablePtr<byte> getHuffmanTable(CoefficientType type, ColorChannelName colorChannelName);

	void applyDCT(ColorChannelName colorChannelName);	
};

typedef std::shared_ptr<Encoder> EncoderPtr;

