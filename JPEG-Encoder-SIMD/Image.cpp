#include "stdafx.h"
#include "SIMD.h"
#include "Image.h"

Image::Image(size_t width, size_t height, SamplingScheme scheme) 
	: width(width), height(height), stepX(8), stepY(8), 
	simulatedWidth(width + (width % stepX == 0 ? 0 : stepX - width % stepX)), 
	simulatedHeight(height + (height % stepY == 0 ? 0 : stepY - height % stepY)),
	samplingScheme(scheme)
{
	channels = std::make_unique<ImageData>(simulatedWidth, simulatedHeight);
	blocksPerChannel[0] = blocksPerChannel[1] = blocksPerChannel[2] = simulatedHeight * simulatedWidth / 8;
}

void Image::setRawPixelDataDirect(float* rgbaData)
{
	int pixelCount = width*height;
	transposeFloatAVX(rgbaData, channels->red(), channels->green(), channels->blue(), pixelCount);
}

void Image::setRawPixelData(float* rgbaData)
{
	// frequently used values
	static const int FLOAT_SIZE = sizeof(float); // 4
	static const int FLOATS_PER_PIXEL = sizeof(PixelData32) / FLOAT_SIZE; // 4
	static const int PIXEL_PER_BLOCK = sizeof(ColorBlock) / FLOAT_SIZE; // 8

	// if the step is 1 we dont have to do any extra stuff
	if (stepX == 1 && stepY == 1) {
		setRawPixelDataDirect(rgbaData);
		return;
	}

	// buffer we fill maually
	float* buffer = new float[(2 * PIXEL_PER_BLOCK + (simulatedWidth - width)) * FLOATS_PER_PIXEL];

	// offsets to keep track of the position in buffers
	int rgbaDataOffsetFloat = 0;		// offset in 'rgbaData'
	int dataPixelOffset = 0;			// offset in 'data'
	int lineOffsetPixel = 0;			// offset in the current line

										// size of the data which should be accessable due to the step size
	int simulatedDataSize = simulatedWidth * simulatedHeight;
	// the size of the real data we have
	int dataSize = width * height * FLOATS_PER_PIXEL;

	while (dataPixelOffset < simulatedDataSize)
	{
		// number of pixels which will be processed directly from the 'rgbaData' array
		int pixelsToProcess = width - lineOffsetPixel;
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
		int pixelsToFillLine = lineRem + simulatedWidth - width;

		int pixelsToFillLineRem = pixelsToFillLine % PIXEL_PER_BLOCK;
		// total number of pixels that be written to 'buffer'
		int pixelsForBuffer = (pixelsToFillLineRem == 0 ? pixelsToFillLine : pixelsToFillLine + PIXEL_PER_BLOCK - pixelsToFillLineRem);
		// update offset
		lineOffsetPixel = (pixelsForBuffer - pixelsToFillLine) % simulatedWidth;

		// total number of float values that be written to 'buffer'
		int floatsForBuffer = pixelsForBuffer * FLOATS_PER_PIXEL;
		for (int bufferFloatOffset = 0; bufferFloatOffset < floatsForBuffer; bufferFloatOffset += FLOATS_PER_PIXEL)
		{
			// if we are on the right side of the pictiure and outside the real data 
			// remove one pixel from the 'rgbaDataOffsetFloat' so we copy the right most pixel
			if ((dataPixelOffset + (bufferFloatOffset) / FLOATS_PER_PIXEL) % simulatedWidth >= width)
				rgbaDataOffsetFloat -= FLOATS_PER_PIXEL;
			// if we are at the end of the real data we have to go one line back to copy the last line
			if (rgbaDataOffsetFloat >= dataSize)
				rgbaDataOffsetFloat -= width * FLOATS_PER_PIXEL;
			memcpy(buffer + bufferFloatOffset, rgbaData + rgbaDataOffsetFloat, FLOATS_PER_PIXEL * FLOAT_SIZE);
			// update offset
			rgbaDataOffsetFloat += FLOATS_PER_PIXEL;
		}
		// if we are at the end of the real data we have to go one line back to copy the last line
		if (rgbaDataOffsetFloat >= dataSize)
			rgbaDataOffsetFloat -= width * FLOATS_PER_PIXEL;

		// transpose the date of the the manually filled buffer
		transposeFloatAVX(buffer, channels->red(dataPixelOffset), channels->green(dataPixelOffset), channels->blue(dataPixelOffset), pixelsForBuffer);
		// update offset
		dataPixelOffset += pixelsForBuffer;
	}

	delete[] buffer;
}

std::vector<float> Image::getRawPixelDataSimulated()
{
	//ImageDataPtr imageData = std::make_shared<std::vector<float>>(simulatedWidth*simulatedHeight*4);
	//transposeFloatAVX_reverse((float*)data, &imageData->operator[](0), simulatedWidth*simulatedHeight);

	std::vector<float> imageData(simulatedWidth*simulatedHeight * 4);
	transposeFloatAVX_reverse(channels->red(), channels->green(), channels->blue(), &imageData[0], simulatedWidth*simulatedHeight);
	return imageData;
}

std::vector<float> Image::getRawPixelData()
{
	if (stepX == 1 && stepY == 1)
	{
		return getRawPixelDataSimulated();
	}

	// TODO: implement

	return std::vector<float>();
}

void Image::SetPixel(uint x, uint y, const PixelData32& color)
{
	size_t pos = simulatedWidth * y + x;

	*channels->red(pos) = color.R;
	*channels->green(pos) = color.G;
	*channels->blue(pos) = color.B;
}

void Image::GetPixel(PixelData32& ref, uint x, uint y) const
{
	size_t pos = simulatedWidth * y + x;

	ref.R = *channels->red(pos);
	ref.G = *channels->green(pos);
	ref.B = *channels->blue(pos);
	ref.A = 1.0f;
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

size_t Image::getWidth() const
{
	return width;
}

size_t Image::getHeight() const
{
	return height;
}

size_t Image::getSimulatedWidth() const
{
	return simulatedWidth;
}

size_t Image::getSimulatedHeight() const
{
	return simulatedHeight;
}