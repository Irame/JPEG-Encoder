#include "stdafx.h"
#include "SIMD.h"
#include "Image.h"

Image::Image(size_t width, size_t height, SamplingScheme scheme) 
	: imageSize(width, height), stepSize(scheme.calcWidthStepSize(), scheme.calcHeightStepSize()),
	simulatedSize(
		width + (width % stepSize.width == 0 ? 0 : stepSize.width - width % stepSize.width),
		height + (height % stepSize.height == 0 ? 0 : stepSize.height - height % stepSize.height)),
	samplingScheme(scheme),
	channelSizes { Dimension2D(simulatedSize) , Dimension2D(simulatedSize) , Dimension2D(simulatedSize) }
{
	channels = std::make_unique<ImageData>(simulatedSize);
	blocksPerChannel[0] = blocksPerChannel[1] = blocksPerChannel[2] = simulatedSize.width * simulatedSize.height / 8;
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
			// if we are on the right side of the pictiure and outside the real data 
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

		// transpose the date of the the manually filled buffer
		transposeFloatAVX(buffer, channels->red(dataPixelOffset), channels->green(dataPixelOffset), channels->blue(dataPixelOffset), pixelsForBuffer);
		// update offset
		dataPixelOffset += pixelsForBuffer;
	}

	delete[] buffer;
}

std::vector<float> Image::getRawPixelDataSimulated()
{
	assert(simulatedSize == channelSizes[0] && simulatedSize == channelSizes[1] && simulatedSize == channelSizes[2]);

	std::vector<float> imageData(simulatedSize.width*simulatedSize.height * 4);
	transposeFloatAVX_reverse(channels->red(), channels->green(), channels->blue(), &imageData[0], simulatedSize.width*simulatedSize.height);
	return imageData;
}

std::vector<float> Image::getRawPixelData()
{
	if (stepSize.width == 1 && stepSize.height == 1)
	{
		return getRawPixelDataSimulated();
	}

	// TODO: implement

	return std::vector<float>();
}

void Image::SetPixel(uint x, uint y, const PixelData32& color)
{
	*channels->red(getPizelPos(0, x, y)) = color.R;
	*channels->green(getPizelPos(1, x, y)) = color.G;
	*channels->blue(getPizelPos(2, x, y)) = color.B;
}

void Image::GetPixel(PixelData32& ref, uint x, uint y) const
{
	ref.R = *channels->red(getPizelPos(0, x, y));
	ref.G = *channels->green(getPizelPos(1, x, y));
	ref.B = *channels->blue(getPizelPos(2, x, y));
	ref.A = 1.0f;
}

inline size_t Image::getPizelPos(int channelIdx, uint x, uint y) const
{
	return channelSizes[channelIdx].width * y + x * simulatedSize.width / channelSizes[channelIdx].width;
}

void Image::convertToYCbCrAVX()
{
	//#pragma omp parallel for
	assert(blocksPerChannel[0] == blocksPerChannel[1] && blocksPerChannel[0] == blocksPerChannel[2]);
	for (size_t i = 0; i < blocksPerChannel[0] * 8; i += 8)
	{
		convertRGBToYCbCrAVXImpl(channels->red(i), channels->green(i), channels->blue(i));
	}
}


void Image::convertToRGBAVX()
{
	//#pragma omp parallel for
	assert(blocksPerChannel[0] == blocksPerChannel[1] && blocksPerChannel[0] == blocksPerChannel[2]);
	for (size_t i = 0; i < blocksPerChannel[0] * 8; i += 8)
	{
		convertYCbCrToRGBAVXImpl(channels->red(i), channels->green(i), channels->blue(i));
	}
}


void Image::applySepiaAVX()
{
	assert(blocksPerChannel[0] == blocksPerChannel[1] && blocksPerChannel[0] == blocksPerChannel[2]);
	for (size_t i = 0; i < blocksPerChannel[0]; i += 8)
	{
		applySepiaFilterAVXImpl(channels->red(i), channels->green(i), channels->blue(i));
	}
}

void Image::multiplyColorChannelByAVX(int colorChannel, float val)
{
	multiplyAVX(channels->getChannel(colorChannel), val, blocksPerChannel[colorChannel] * 8);
}

void Image::reduceWidthResolutionColorChannel(int channelIdx, int factor, ReductionMethod method)
{
	if (factor == 1) return;

	float* channel = channels->getChannel(channelIdx);
	size_t channelDataSize = blocksPerChannel[channelIdx] * 8;

	blocksPerChannel[channelIdx] /= factor;
	channelSizes[channelIdx].height /= factor;

	//TODO: implement AVX code paths

	if (method == Subsampling)
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
	if (factor == 1) return;

	float* channel = channels->getChannel(channelIdx);
	size_t channelDataSize = blocksPerChannel[channelIdx] * 8;

	size_t newChannelDataSize = channelDataSize / factor;
	blocksPerChannel[channelIdx] /= factor;

	Dimension2D oldchannelSize = channelSizes[channelIdx];

	size_t newChannelHeight = oldchannelSize.height / factor;
	channelSizes[channelIdx].height = newChannelHeight;

	//TODO: implement AVX code paths

	if (method == Average && factor == 2)
	{
		size_t srcOffset = 0, dstOffset = 0;
		while (srcOffset < channelDataSize + oldchannelSize.width)
		{
			srcOffset %= channelDataSize;
			dstOffset %= newChannelDataSize;

			halfHeightResolutionAverageAVX(&channel[srcOffset], &channel[srcOffset += oldchannelSize.width], &channel[dstOffset]);

			srcOffset += oldchannelSize.width;
			dstOffset += oldchannelSize.width;

			if (srcOffset / oldchannelSize.width == oldchannelSize.height) srcOffset += 8;
			if (srcOffset / oldchannelSize.width == newChannelHeight) srcOffset += 8;
		}
	} 
	else if (method == Subsampling) 
	{
		size_t srcOffset = 0, dstOffset = 0;
		while (srcOffset < channelDataSize + oldchannelSize.width)
		{
			srcOffset %= channelDataSize;
			dstOffset %= newChannelDataSize;

			channel[dstOffset] = channel[srcOffset];

			srcOffset += oldchannelSize.width * factor;
			dstOffset += oldchannelSize.width;

			if (srcOffset / oldchannelSize.width == oldchannelSize.height) srcOffset++;
			if (srcOffset / oldchannelSize.width == newChannelHeight) srcOffset++;
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
			if (srcOffset / oldchannelSize.width == newChannelHeight) srcOffset++;
		}
	}
}

const Dimension2D& Image::getImageSize() const
{
	return imageSize;
}

const Dimension2D& Image::getSimulatedSize() const
{
	return simulatedSize;
}