#include "stdafx.h"
#include "Image.h"

#include "SIMD.h"
#include <algorithm>

Image::Image(size_t width, size_t height, size_t stepY = 1, size_t stepX = 1)
	: data(nullptr), width(width), height(height), slots(0), stepX(stepX), stepY(stepY)
{
	int widthStepRem = width % stepX;
	int heightStepRem = height % stepY;
	simulatedWidth = widthStepRem == 0 ? width : width + stepX - widthStepRem;
	simulatedHeight = heightStepRem == 0 ? height : height + stepY - heightStepRem;
	slots = static_cast<uint>(ceil((simulatedWidth * simulatedHeight) / 8.0));
	data = (PixelData32T8*)_mm_malloc(sizeof(PixelData32T8)*slots, 64);
}

Image::~Image()
{
	_mm_free(data);
}

void Image::setRawPixelDataDirect(float* rgbaData)
{
	int pixelCount = width*height;
	transposeFloatAVX(rgbaData, (float*)data, pixelCount);
}

void Image::setRawPixelData(float* rgbaData)
{
	// frequently used values
	static const int FLOAT_SIZE = sizeof(float); // 4
	static const int FLOATS_PER_PIXEL = sizeof(PixelData32) / FLOAT_SIZE; // 4
	static const int PIXEL_PER_BLOCK = sizeof(PixelData32T8) / FLOATS_PER_PIXEL / FLOAT_SIZE; // 8

	// if the step is 1 we dont have to do any extra stuff
	if (stepX == 1 && stepY == 1) {
		setRawPixelDataDirect(rgbaData);
		return;
	}

	// buffer we fill maually
	float* buffer = new float[(2 * PIXEL_PER_BLOCK + (simulatedWidth-width)) * FLOATS_PER_PIXEL];

	// offsets to keep track of the position in buffers
	int rgbaDataOffsetFloat = 0;		// offset in 'rgbaData'
	int dataOffsetFloat = 0;			// offset in 'data'
	int lineOffsetPixel = 0;			// offset in the current line

	// size of the data which should be accessable due to the step size
	int simulatedDataSize = simulatedWidth * simulatedHeight * FLOATS_PER_PIXEL;
	// the size of the real data we have
	int dataSize = width * height * FLOATS_PER_PIXEL;

	while (dataOffsetFloat < simulatedDataSize)
	{
		// number of pixels which will be processed directly from the 'rgbaData' array
		int pixelsToProcess = width - lineOffsetPixel;
		// pixels that are in the last block (or 0)
		int lineRem = pixelsToProcess % PIXEL_PER_BLOCK;
		
		// adjust pixels to process so that they align with the block size (8)
		pixelsToProcess -= lineRem;
		// process the pixels directly from 'rgbaData'
		transposeFloatAVX(rgbaData + rgbaDataOffsetFloat, (float*)data + dataOffsetFloat, pixelsToProcess);
		// update offsets
		rgbaDataOffsetFloat += pixelsToProcess * FLOATS_PER_PIXEL;
		dataOffsetFloat += pixelsToProcess * FLOATS_PER_PIXEL;
		
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
			if (((dataOffsetFloat + bufferFloatOffset) / FLOATS_PER_PIXEL) % simulatedWidth >= width)
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
		transposeFloatAVX(buffer, (float*)data + dataOffsetFloat, pixelsForBuffer);
		// update offset
		dataOffsetFloat += floatsForBuffer;
	}

	delete[] buffer;
}

std::vector<float> Image::getRawPixelDataSimulated()
{
	//ImageDataPtr imageData = std::make_shared<std::vector<float>>(simulatedWidth*simulatedHeight*4);
	//transposeFloatAVX_reverse((float*)data, &imageData->operator[](0), simulatedWidth*simulatedHeight);

	std::vector<float> imageData(simulatedWidth*simulatedHeight * 4);
	transposeFloatAVX_reverse((float*)data, &imageData[0], simulatedWidth*simulatedHeight);
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

void Image::SetPixel(uint x, uint y, PixelData32 color)
{
	PixelPos pos = GetPixelPos(x, y);

	data[pos.slot].red[pos.index] = color.R;
	data[pos.slot].green[pos.index] = color.G;
	data[pos.slot].blue[pos.index] = color.B;
	data[pos.slot].alpha[pos.index] = color.A;
}

void Image::GetPixel(PixelData32& ref, uint x, uint y)
{
	PixelPos pos = GetPixelPos(x, y);

	ref.R = data[pos.slot].red[pos.index];
	ref.G = data[pos.slot].green[pos.index];
	ref.B = data[pos.slot].blue[pos.index];
	ref.A = data[pos.slot].alpha[pos.index];
}

inline PixelPos Image::GetPixelPos(uint x, uint y)
{
	div_t tmp = div(y*simulatedWidth + x, 8);
	return PixelPos(tmp.quot, tmp.rem);
}

void Image::applySepia()
{
	for (size_t i = 0; i < this->slots; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			data[i].red[j]   = 0.393f*data[i].red[j] + 0.769f*data[i].green[j] + 0.189f*data[i].blue[j];
			data[i].green[j] = 0.349f*data[i].red[j] + 0.686f*data[i].green[j] + 0.168f*data[i].blue[j];
			data[i].blue[j]  = 0.272f*data[i].red[j] + 0.534f*data[i].green[j] + 0.131f*data[i].blue[j];
			data[i].alpha[j] = data[i].alpha[j];
			if (data[i].red[j]   > 255) data[i].red[j]   = 255;
			if (data[i].green[j] > 255) data[i].green[j] = 255;
			if (data[i].blue[j]  > 255) data[i].blue[j]  = 255;
		}
	}
}

void Image::convertToYCbCr()
{
	for (size_t i = 0; i < this->slots; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			data[i].red[j]   = 0.0f +  0.2990f * data[i].red[j] +  0.5870f * data[i].green[j] +  0.1140f * data[i].blue[j];
			data[i].green[j] = 0.5f + -0.1687f * data[i].red[j] + -0.3312f * data[i].green[j] +  0.5000f * data[i].blue[j];
			data[i].blue[j]  = 0.5f +  0.5000f * data[i].red[j] + -0.4186f * data[i].green[j] + -0.0813f * data[i].blue[j];
			data[i].alpha[j] = 1.0f;
		}
	}
}

void Image::convertToRGB()
{
	for (size_t i = 0; i < this->slots; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			data[i].red[j] = 1.0f * (data[i].red[j] - 0.0f) + 0.0f     * (data[i].green[j] - 0.5f) + 1.40209f * (data[i].blue[j] - 0.5f);
			data[i].green[j] = 1.0f * (data[i].red[j] - 0.0f) + -0.34415f * (data[i].green[j] - 0.5f) + -0.71418f * (data[i].blue[j] - 0.5f);
			data[i].blue[j] = 1.0f * (data[i].red[j] - 0.0f) + 1.77204f * (data[i].green[j] - 0.5f) + 0.0f     * (data[i].blue[j] - 0.5f);
			data[i].alpha[j] = 1.0f;
		}
	}
}


void Image::convertToYCbCrAVX()
{
//#pragma omp parallel for
	for (size_t i = 0; i < this->slots; i++)
	{
		convertRGBToYCbCrAVXImpl(data[i]);
	}
}


void Image::convertToRGBAVX()
{
//#pragma omp parallel for
	for (size_t i = 0; i < this->slots; i++)
	{
		convertYCbCrToRGBAVXImpl(data[i]);
	}
}


void Image::applySepiaAVX()
{
	for (size_t i = 0; i < this->slots; i++)
	{
		applySepiaFilterAVXImpl(data[i]);
	}
}

void Image::multiplyColorChannelByAVX(int colorChannel, float val)
{
	multiplyAVX((float*)data + colorChannel * 8, val, this->slots * 32);
}