#pragma once
#include "Image.h"

class Encoder : public Image
{
	void reduceWidthResolutionColorChannel(ColorChannelName channelIdx, int factor, ReductionMethod method);
	void reduceHeightResolutionColorChannel(ColorChannelName channelIdx, int factor, ReductionMethod method);

	std::vector<PointerMatrix> getBlocks(ColorChannelName colorChannelName) const;

public:
	Encoder(Image image);


	void convertToYCbCr();
	void convertToRGB();

	void multiplyColorChannelBy(ColorChannelName colorChannel, float val);

	void reduceResolutionBySchema();

	HuffmanTablePtr<byte> getHuffmanTable(ColorChannelName colorChannelName) const;
};

typedef std::shared_ptr<Encoder> EncoderPtr;

