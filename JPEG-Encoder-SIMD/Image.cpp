#include "stdafx.h"
#include "SIMD.h"
#include "Image.h"
#include "HuffmanCoding.h"

Image::Image(size_t width, size_t height, SamplingScheme scheme, QTable luminance, QTable chrominance)
	: imageSize(width, height), stepSize(scheme.calcWidthStepSize(), scheme.calcHeightStepSize()),
	simulatedSize(
		width + (width % stepSize.width == 0 ? 0 : stepSize.width - width % stepSize.width),
		height + (height % stepSize.height == 0 ? 0 : stepSize.height - height % stepSize.height)),
	channelSizes { Dimension2D(simulatedSize) , Dimension2D(simulatedSize) , Dimension2D(simulatedSize) },
	samplingScheme(scheme),
	luminance(luminance),
	chrominance(chrominance)
{
	channels = std::make_unique<ImageData>(simulatedSize);
	blocksPerChannel[0] = blocksPerChannel[1] = blocksPerChannel[2] = simulatedSize.width * simulatedSize.height / 8;
}

Image::Image(Image& origImage)
	: imageSize(origImage.imageSize), stepSize(origImage.stepSize), simulatedSize(origImage.simulatedSize), 
	channelSizes{ origImage.channelSizes[0], origImage.channelSizes[1], origImage.channelSizes[2] }, 
	samplingScheme(origImage.samplingScheme), luminance(origImage.luminance), chrominance(origImage.chrominance)
{
	memcpy(blocksPerChannel, origImage.blocksPerChannel, sizeof(blocksPerChannel));
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
	size_t pixelCount = imageSize.width * imageSize.height;
	transposeFloatAVX(rgbaData, channels->red(), channels->green(), channels->blue(), pixelCount);
}

void Image::setRawPixelData(float* rgbaData)
{
	// frequently used values
	static const size_t FLOAT_SIZE = sizeof(float); // 4
	static const size_t FLOATS_PER_PIXEL = sizeof(PixelData32) / FLOAT_SIZE; // 4
	static const size_t PIXEL_PER_BLOCK = sizeof(ColorBlock) / FLOAT_SIZE; // 8

	// if the step is 1 we dont have to do any extra stuff
	if (stepSize.width == 1 && stepSize.height == 1) {
		setRawPixelDataDirect(rgbaData);
		return;
	}

	// buffer we fill maually
	float* buffer = new float[(2 * PIXEL_PER_BLOCK + (simulatedSize.width - imageSize.width)) * FLOATS_PER_PIXEL];

	// offsets to keep track of the position in buffers
	size_t rgbaDataOffsetFloat = 0;		// offset in 'rgbaData'
	size_t dataPixelOffset = 0;			// offset in 'data'
	size_t lineOffsetPixel = 0;			// offset in the current line

	// size of the data which should be accessable due to the step size
	size_t simulatedDataSize = simulatedSize.width * simulatedSize.height;
	// the size of the real data we have
	size_t dataSize = imageSize.width * imageSize.height * FLOATS_PER_PIXEL;

	while (dataPixelOffset < simulatedDataSize)
	{
		// number of pixels which will be processed directly from the 'rgbaData' array
		size_t pixelsToProcess = imageSize.width - lineOffsetPixel;
		// pixels that are in the last block (or 0)
		size_t lineRem = pixelsToProcess % PIXEL_PER_BLOCK;

		// adjust pixels to process so that they align with the block size (8)
		pixelsToProcess -= lineRem;
		// process the pixels directly from 'rgbaData'
		transposeFloatAVX(rgbaData + rgbaDataOffsetFloat, channels->red(dataPixelOffset), channels->green(dataPixelOffset), channels->blue(dataPixelOffset), pixelsToProcess);
		// update offsets
		rgbaDataOffsetFloat += pixelsToProcess * FLOATS_PER_PIXEL;
		dataPixelOffset += pixelsToProcess;

		// pixels which are needed to fill one line to a total width of 'simulatedWidth'
		size_t pixelsToFillLine = lineRem + simulatedSize.width - imageSize.width;

		size_t pixelsToFillLineRem = pixelsToFillLine % PIXEL_PER_BLOCK;
		// total number of pixels that be written to 'buffer'
		size_t pixelsForBuffer = (pixelsToFillLineRem == 0 ? pixelsToFillLine : pixelsToFillLine + PIXEL_PER_BLOCK - pixelsToFillLineRem);
		// update offset
		lineOffsetPixel = (pixelsForBuffer - pixelsToFillLine) % simulatedSize.width;

		// total number of float values that be written to 'buffer'
		size_t floatsForBuffer = pixelsForBuffer * FLOATS_PER_PIXEL;
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

inline size_t Image::getPixelPos(ColorChannelName channelIdx, size_t x, size_t y) const
{
	return channelSizes[channelIdx].width * (y * channelSizes[channelIdx].height / simulatedSize.height)
		+ x * channelSizes[channelIdx].width / simulatedSize.width;
}

void Image::setPixel(size_t x, size_t y, const PixelData32& color)
{
	*channels->red(getPixelPos(R, x, y)) = color.R;
	*channels->green(getPixelPos(G, x, y)) = color.G;
	*channels->blue(getPixelPos(B, x, y)) = color.B;
}

void Image::getPixel(PixelData32& ref, size_t x, size_t y) const
{
	ref.R = *channels->red(getPixelPos(R, x, y));
	ref.G = *channels->green(getPixelPos(G, x, y));
	ref.B = *channels->blue(getPixelPos(B, x, y));
	ref.A = 1.0f;
}


void Image::applySepia()
{
	assert(blocksPerChannel[0] == blocksPerChannel[1] && blocksPerChannel[0] == blocksPerChannel[2]);
	for (size_t i = 0; i < blocksPerChannel[0]; i += 8)
	{
		applySepiaFilterAVXImpl(channels->red(i), channels->green(i), channels->blue(i));
	}
}