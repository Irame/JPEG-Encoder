#include "stdafx.h"
#include "SIMD.h"
#include "Image.h"

Image::Image(size_t width, size_t height, SamplingScheme scheme) 
	: imageSize(width, height), stepSize(scheme.calcWidthStepSize(), scheme.calcHeightStepSize()),
	simulatedSize(
		width + (width % stepSize.width == 0 ? 0 : stepSize.width - width % stepSize.width),
		height + (height % stepSize.height == 0 ? 0 : stepSize.height - height % stepSize.height)),
	channelSizes { Dimension2D(simulatedSize) , Dimension2D(simulatedSize) , Dimension2D(simulatedSize) },
	samplingScheme(scheme)
{
	channels = std::make_unique<ImageData>(simulatedSize);
	blocksPerChannel[0] = blocksPerChannel[1] = blocksPerChannel[2] = simulatedSize.width * simulatedSize.height / 8;
}

const Dimension2D& Image::getImageSize() const
{
	return imageSize;
}

const Dimension2D& Image::getSimulatedSize() const
{
	return simulatedSize;
}

void Image::setRawPixelDataDirect(float* rgbaData)
{
	int pixelCount = imageSize.width * imageSize.height;
	transposeFloatAVX(rgbaData, channels->red(), channels->green(), channels->blue(), pixelCount);
}

void Image::setRawPixelData(float* rgbaData)
{
	// frequently used values
	static const int FLOAT_SIZE = sizeof(float); // 4
	static const int FLOATS_PER_PIXEL = sizeof(PixelData32) / FLOAT_SIZE; // 4
	static const int PIXEL_PER_BLOCK = sizeof(ColorBlock) / FLOAT_SIZE; // 8

	// if the step is 1 we dont have to do any extra stuff
	if (stepSize.width == 1 && stepSize.height == 1) {
		setRawPixelDataDirect(rgbaData);
		return;
	}

	// buffer we fill maually
	float* buffer = new float[(2 * PIXEL_PER_BLOCK + (simulatedSize.width - imageSize.width)) * FLOATS_PER_PIXEL];

	// offsets to keep track of the position in buffers
	int rgbaDataOffsetFloat = 0;		// offset in 'rgbaData'
	int dataPixelOffset = 0;			// offset in 'data'
	int lineOffsetPixel = 0;			// offset in the current line

	// size of the data which should be accessable due to the step size
	int simulatedDataSize = simulatedSize.width * simulatedSize.height;
	// the size of the real data we have
	int dataSize = imageSize.width * imageSize.height * FLOATS_PER_PIXEL;

	while (dataPixelOffset < simulatedDataSize)
	{
		// number of pixels which will be processed directly from the 'rgbaData' array
		int pixelsToProcess = imageSize.width - lineOffsetPixel;
		// pixels that are in the last block (or 0)
		int lineRem = pixelsToProcess % PIXEL_PER_BLOCK;

		// adjust pixels to process so that they align with the block size (8)
		pixelsToProcess -= lineRem;
		// process the pixels directly from 'rgbaData'
		transposeFloatAVX(rgbaData + rgbaDataOffsetFloat, channels->red(dataPixelOffset), channels->green(dataPixelOffset), channels->blue(dataPixelOffset), pixelsToProcess);
		// update offsets
		rgbaDataOffsetFloat += pixelsToProcess * FLOATS_PER_PIXEL;
		dataPixelOffset += pixelsToProcess;

		// pixels which are needed to fill one line to a total width of 'simulatedWidth'
		int pixelsToFillLine = lineRem + simulatedSize.width - imageSize.width;

		int pixelsToFillLineRem = pixelsToFillLine % PIXEL_PER_BLOCK;
		// total number of pixels that be written to 'buffer'
		int pixelsForBuffer = (pixelsToFillLineRem == 0 ? pixelsToFillLine : pixelsToFillLine + PIXEL_PER_BLOCK - pixelsToFillLineRem);
		// update offset
		lineOffsetPixel = (pixelsForBuffer - pixelsToFillLine) % simulatedSize.width;

		// total number of float values that be written to 'buffer'
		int floatsForBuffer = pixelsForBuffer * FLOATS_PER_PIXEL;
		for (int bufferFloatOffset = 0; bufferFloatOffset < floatsForBuffer; bufferFloatOffset += FLOATS_PER_PIXEL)
		{
			// if we are on the right side of the picture and outside the real data 
			// remove one pixel from the 'rgbaDataOffsetFloat' so we copy the right most pixel
			if ((dataPixelOffset + (bufferFloatOffset) / FLOATS_PER_PIXEL) % simulatedSize.width >= imageSize.width)
				rgbaDataOffsetFloat -= FLOATS_PER_PIXEL;
			// if we are at the end of the real data we have to go one line back to copy the last line
			if (rgbaDataOffsetFloat >= dataSize)
				rgbaDataOffsetFloat -= imageSize.width * FLOATS_PER_PIXEL;
			memcpy(buffer + bufferFloatOffset, rgbaData + rgbaDataOffsetFloat, FLOATS_PER_PIXEL * FLOAT_SIZE);
			// update offset
			rgbaDataOffsetFloat += FLOATS_PER_PIXEL;
		}
		// if we are at the end of the real data we have to go one line back to copy the last line
		if (rgbaDataOffsetFloat >= dataSize)
			rgbaDataOffsetFloat -= imageSize.width * FLOATS_PER_PIXEL;

		// transpose the data of the the manually filled buffer
		transposeFloatAVX(buffer, channels->red(dataPixelOffset), channels->green(dataPixelOffset), channels->blue(dataPixelOffset), pixelsForBuffer);
		// update offset
		dataPixelOffset += pixelsForBuffer;
	}

	delete[] buffer;
}

std::vector<float> Image::getRawPixelDataSimulated()
{
	std::vector<float> imageData(simulatedSize.width*simulatedSize.height * 4);

	if (simulatedSize == channelSizes[0] && simulatedSize == channelSizes[1] && simulatedSize == channelSizes[2]) {
		transposeFloatAVX_reverse(channels->red(), channels->green(), channels->blue(), &imageData[0], simulatedSize.width*simulatedSize.height);
	} 
	else // easy non AVX implementation
	{
		size_t offset = 0;
		PixelData32 pixel;
		for (uint y = 0; y < simulatedSize.height; y++)
		{
			for (uint x = 0; x < simulatedSize.width; x++)
			{
				getPixel(pixel, x, y);
				memcpy(imageData.data() + offset, &pixel, sizeof(PixelData32));
				offset += sizeof(PixelData32) / sizeof(float);
			}
		}
	}

	return imageData;
}

std::vector<float> Image::getRawPixelData()
{
	if (stepSize.width == 1 && stepSize.height == 1)
	{
		return getRawPixelDataSimulated();
	}
	else // easy non AVX implementation
	{
		std::vector<float> imageData(imageSize.width*imageSize.height * 4);
		size_t offset = 0;
		PixelData32 pixel;
		for (uint y = 0; y < imageSize.height; y++)
		{
			for (uint x = 0; x < imageSize.width; x++)
			{
				getPixel(pixel, x, y);
				memcpy(imageData.data() + offset, &pixel, sizeof(PixelData32));
				offset += sizeof(PixelData32) / sizeof(float);
			}
		}
		return imageData;
	}
}

inline size_t Image::getPixelPos(int channelIdx, uint x, uint y) const
{
	return channelSizes[channelIdx].width * (y * channelSizes[channelIdx].height / simulatedSize.height)
		+ x * channelSizes[channelIdx].width / simulatedSize.width;
}

void Image::setPixel(uint x, uint y, const PixelData32& color)
{
	*channels->red(getPixelPos(0, x, y)) = color.R;
	*channels->green(getPixelPos(1, x, y)) = color.G;
	*channels->blue(getPixelPos(2, x, y)) = color.B;
}

void Image::getPixel(PixelData32& ref, uint x, uint y) const
{
	ref.R = *channels->red(getPixelPos(0, x, y));
	ref.G = *channels->green(getPixelPos(1, x, y));
	ref.B = *channels->blue(getPixelPos(2, x, y));
	ref.A = 1.0f;
}

void Image::convertToYCbCr()
{
	//#pragma omp parallel for
	assert(blocksPerChannel[0] == blocksPerChannel[1] && blocksPerChannel[0] == blocksPerChannel[2]);
	for (size_t i = 0; i < blocksPerChannel[0] * 8; i += 8)
	{
		convertRGBToYCbCrAVXImpl(channels->red(i), channels->green(i), channels->blue(i));
	}
}


void Image::convertToRGB()
{
	//#pragma omp parallel for
	assert(blocksPerChannel[0] == blocksPerChannel[1] && blocksPerChannel[0] == blocksPerChannel[2]);
	for (size_t i = 0; i < blocksPerChannel[0] * 8; i += 8)
	{
		convertYCbCrToRGBAVXImpl(channels->red(i), channels->green(i), channels->blue(i));
	}
}


void Image::applySepia()
{
	assert(blocksPerChannel[0] == blocksPerChannel[1] && blocksPerChannel[0] == blocksPerChannel[2]);
	for (size_t i = 0; i < blocksPerChannel[0]; i += 8)
	{
		applySepiaFilterAVXImpl(channels->red(i), channels->green(i), channels->blue(i));
	}
}

void Image::multiplyColorChannelBy(int colorChannel, float val)
{
	multiplyAVX(channels->getChannel(colorChannel), val, blocksPerChannel[colorChannel] * 8);
}

void Image::reduceWidthResolutionColorChannel(int channelIdx, int factor, ReductionMethod method)
{
	assert(channelSizes[channelIdx].width % factor == 0);

	// factor 1 => no reduction necessary
	if (factor == 1) return;

	// get channel data and information
	float* channel = channels->getChannel(channelIdx);
	size_t channelDataSize = blocksPerChannel[channelIdx] * 8;

	// adjust the channel information
	blocksPerChannel[channelIdx] /= factor;
	channelSizes[channelIdx].width /= factor;

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

void Image::reduceHeightResolutionColorChannel(int channelIdx, int factor, ReductionMethod method)
{
	assert(channelSizes[channelIdx].height % factor == 0);

	// factor 1 => no reduction necessary
	if (factor == 1) return;

	// get channel data and information
	float* channel = channels->getChannel(channelIdx);
	size_t channelDataSize = blocksPerChannel[channelIdx] * 8;

	// save old channel info
	Dimension2D oldchannelSize = channelSizes[channelIdx];

	// calc new channel info
	size_t newChannelDataSize = channelDataSize / factor;
	size_t newChannelHeight = oldchannelSize.height / factor;
	blocksPerChannel[channelIdx] /= factor;
	channelSizes[channelIdx].height = newChannelHeight;

	//TODO: implement AVX code paths

	// use special AVX codepath for higher perfomance if the factor is 2 and method is Average
	if (method == Average && factor == 2)
	{
		size_t srcOffset = 0, dstOffset = 0;
		while (srcOffset < channelDataSize + oldchannelSize.width)
		{
			srcOffset %= channelDataSize;
			dstOffset %= newChannelDataSize;

			halfHeightResolutionAverageAVX(&channel[srcOffset], &channel[srcOffset + oldchannelSize.width], &channel[dstOffset]);

			srcOffset += oldchannelSize.width * 2;
			dstOffset += oldchannelSize.width;

			if (srcOffset / oldchannelSize.width == oldchannelSize.height) srcOffset += 8;
			if (dstOffset / oldchannelSize.width == newChannelHeight) dstOffset += 8;
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
		size_t srcOffset = 0, dstOffset = 0, valCount = 0;
		while (srcOffset < channelDataSize + oldchannelSize.width)
		{
			srcOffset %= channelDataSize;
			dstOffset %= newChannelDataSize;

			sum += channel[srcOffset];
			valCount++;
			if (valCount == factor)
			{
				channel[dstOffset] = sum / factor;
				sum = 0.0f;
				valCount = 0;
				dstOffset += oldchannelSize.width;
			}
			srcOffset += oldchannelSize.width;

			if (srcOffset / oldchannelSize.width == oldchannelSize.height) srcOffset++;
			if (dstOffset / oldchannelSize.width == newChannelHeight) dstOffset++;
		}
	}
}

void Image::reduceResolutionBySchema()
{
	reduceWidthResolutionColorChannel(0, samplingScheme.yReductionOptions.widthFactor, samplingScheme.yReductionOptions.widthMethod);
	reduceWidthResolutionColorChannel(1, samplingScheme.cbReductionOptions.widthFactor, samplingScheme.cbReductionOptions.widthMethod);
	reduceWidthResolutionColorChannel(2, samplingScheme.crReductionOptions.widthFactor, samplingScheme.crReductionOptions.widthMethod);

	reduceHeightResolutionColorChannel(0, samplingScheme.yReductionOptions.heightFactor, samplingScheme.yReductionOptions.heightMethod);
	reduceHeightResolutionColorChannel(1, samplingScheme.cbReductionOptions.heightFactor, samplingScheme.cbReductionOptions.heightMethod);
	reduceHeightResolutionColorChannel(2, samplingScheme.crReductionOptions.heightFactor, samplingScheme.crReductionOptions.heightMethod);
}