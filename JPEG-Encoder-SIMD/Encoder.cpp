#include "stdafx.h"
#include "Encoder.h"
#include "SIMD.h"


Encoder::Encoder(Image image) : Image(image)
{}

void Encoder::convertToYCbCr()
{
	//#pragma omp parallel for
	assert(blocksPerChannel[0] == blocksPerChannel[1] && blocksPerChannel[0] == blocksPerChannel[2]);
	for (size_t i = 0; i < blocksPerChannel[0] * 8; i += 8)
	{
		convertRGBToYCbCrAVXImpl(channels->red(i), channels->green(i), channels->blue(i));
	}
}

void Encoder::convertToRGB()
{
	//#pragma omp parallel for
	assert(blocksPerChannel[0] == blocksPerChannel[1] && blocksPerChannel[0] == blocksPerChannel[2]);
	for (size_t i = 0; i < blocksPerChannel[0] * 8; i += 8)
	{
		convertYCbCrToRGBAVXImpl(channels->red(i), channels->green(i), channels->blue(i));
	}
}

void Encoder::multiplyColorChannelBy(ColorChannelName colorChannel, float val)
{
	multiplyAVX(channels->getChannel(colorChannel), val, blocksPerChannel[colorChannel] * 8);
}

void Encoder::reduceWidthResolutionColorChannel(ColorChannelName channelName, int factor, ReductionMethod method)
{
	assert(channelSizes[channelName].width % factor == 0);

	// factor 1 => no reduction necessary
	if (factor == 1) return;

	// get channel data and information
	float* channel = channels->getChannel(channelName);
	size_t channelDataSize = blocksPerChannel[channelName] * 8;

	// adjust the channel information
	blocksPerChannel[channelName] /= factor;
	channelSizes[channelName].width /= factor;

	//TODO: implement AVX code paths

	// use special AVX codepath for higher perfomance if the factor is 2
	if (method == Average && factor == 2)
	{
		for (size_t srcOffset = 0, dstOffset = 0; srcOffset < channelDataSize; )
		{
			halfWidthResolutionAverageAVX(&channel[srcOffset], &channel[srcOffset + 8], &channel[dstOffset]);
			srcOffset += 16;
			dstOffset += 8;
		}
	}
	else if (method == Subsampling && factor == 2)
	{
		for (size_t srcOffset = 0, dstOffset = 0; srcOffset < channelDataSize; )
		{
			halfWidthResolutionSubsamplingAVX(&channel[srcOffset], &channel[srcOffset + 8], &channel[dstOffset]);
			srcOffset += 16;
			dstOffset += 8;
		}
	}
	// otherwise use a general implementation
	else if (method == Subsampling)
	{
		for (size_t srcOffset = 0, dstOffset = 0; srcOffset < channelDataSize; srcOffset += factor)
		{
			channel[dstOffset++] = channel[srcOffset];
		}
	}
	else if (method == Average)
	{
		float sum = 0.0f;
		for (size_t srcOffset = 0, dstOffset = 0; srcOffset < channelDataSize; )
		{
			sum += channel[srcOffset++];
			if (srcOffset % factor == 0) {
				channel[dstOffset++] = sum / factor;
				sum = 0.0f;
			}
		}
	}
}


void Encoder::reduceHeightResolutionColorChannel(ColorChannelName channelName, int factor, ReductionMethod method)
{
	assert(channelSizes[channelName].height % factor == 0);

	// factor 1 => no reduction necessary
	if (factor == 1) return;

	// get channel data and information
	float* channel = channels->getChannel(channelName);
	size_t channelDataSize = blocksPerChannel[channelName] * 8;

	// save old channel info
	Dimension2D oldchannelSize = channelSizes[channelName];

	// calc new channel info
	size_t newChannelDataSize = channelDataSize / factor;
	size_t newChannelHeight = oldchannelSize.height / factor;
	blocksPerChannel[channelName] /= factor;
	channelSizes[channelName].height = newChannelHeight;

	//TODO: implement AVX code paths

	// use special AVX codepath for higher perfomance if the factor is 2 and method is Average
	if (method == Average && factor == 2)
	{
		// Processes the image in columns of width 8 from top to bottom
		for (size_t x = 0; x < oldchannelSize.width; x += 8) {
			for (size_t srcOffset = x, destOffset = x; srcOffset < channelDataSize; srcOffset += oldchannelSize.width * 2, destOffset += oldchannelSize.width) {
				halfHeightResolutionAverageAVX(&channel[srcOffset], &channel[srcOffset + oldchannelSize.width], &channel[destOffset]);
			}
		}
	}
	// otherwise use a general implementation
	else if (method == Subsampling)
	{
		size_t srcOffset = 0, dstOffset = 0;
		while (srcOffset < channelDataSize)
		{
			memcpy(&channel[dstOffset], &channel[srcOffset], oldchannelSize.width * sizeof(float));

			srcOffset += oldchannelSize.width * factor;
			dstOffset += oldchannelSize.width;
		}
	}
	else if (method == Average)
	{
		float sum = 0.0f;
		size_t valCount = 0;
		for (size_t x = 0; x < oldchannelSize.width; x++) {
			for (size_t srcOffset = x, destOffset = x; srcOffset < channelDataSize; srcOffset += oldchannelSize.width) {
				sum += channel[srcOffset];
				valCount++;
				if (valCount == factor)
				{
					channel[destOffset] = sum / factor;
					destOffset += oldchannelSize.width;
					sum = 0.0f;
					valCount = 0;
				}
			}
		}
	}
}

void Encoder::reduceResolutionBySchema()
{
	reduceWidthResolutionColorChannel(Y, samplingScheme.yReductionOptions.widthFactor, samplingScheme.yReductionOptions.widthMethod);
	reduceWidthResolutionColorChannel(Cb, samplingScheme.cbReductionOptions.widthFactor, samplingScheme.cbReductionOptions.widthMethod);
	reduceWidthResolutionColorChannel(Cr, samplingScheme.crReductionOptions.widthFactor, samplingScheme.crReductionOptions.widthMethod);

	reduceHeightResolutionColorChannel(Y, samplingScheme.yReductionOptions.heightFactor, samplingScheme.yReductionOptions.heightMethod);
	reduceHeightResolutionColorChannel(Cb, samplingScheme.cbReductionOptions.heightFactor, samplingScheme.cbReductionOptions.heightMethod);
	reduceHeightResolutionColorChannel(Cr, samplingScheme.crReductionOptions.heightFactor, samplingScheme.crReductionOptions.heightMethod);
}

HuffmanTablePtr<byte> Encoder::getHuffmanTable(ColorChannelName colorChannelName) const
{
	//std::vector<PointerMatrix> blocks = getBlocks(colorChannelName);
	//for (PointerMatrix& matrix : blocks) {
	//	twoDimensionalDCTandQuantisationAVX(matrix, qTables[colorChannelName], matrix);
	//}
	std::vector<byte> allSymbols{ 0, 1, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6 };
	return HuffmanTable<byte>::create(16, allSymbols);
}

std::vector<PointerMatrix> Encoder::getBlocks(ColorChannelName colorChannelName) const
{
	float* channel = channels->getChannel(colorChannelName);
	size_t height = channelSizes[colorChannelName].height;
	size_t width = channelSizes[colorChannelName].width;

	std::vector<PointerMatrix> blocks;
	blocks.reserve(height*width / 8 * 8);

	for (int y = 0; y < height; y += 8) {
		for (int x = 0; x < width; x += 8) {
			blocks.emplace_back(
				channel + x + width * 0 + height * y,
				channel + x + width * 1 + height * y,
				channel + x + width * 2 + height * y,
				channel + x + width * 3 + height * y,
				channel + x + width * 4 + height * y,
				channel + x + width * 5 + height * y,
				channel + x + width * 6 + height * y,
				channel + x + width * 7 + height * y
				);
		}
	}

	return blocks;
}