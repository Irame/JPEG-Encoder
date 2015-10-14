#include "stdafx.h"
#include "Image.h"

#include "SIMD.h"

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

void Image::setRawPixelData2(float* rgbaData)
{
	int pixelCount = width*height;
	int rem = pixelCount % 8;
	int pixelForAVX = pixelCount - rem;
	transposeFloatAVX(rgbaData, (float*)data, pixelForAVX);
	PixelData32T8 tail;
	memset(&tail, 0, sizeof(PixelData32T8));
	memcpy(&tail, rgbaData + pixelForAVX * 4, rem * 4 * sizeof(float));
	transposeFloatAVX((float*)&tail, ((float*)data) + pixelForAVX * 4, 8);
}

void Image::setRawPixelData(float* rgbaData)
{
	static const int FLOAT_SIZE = sizeof(float); // 4
	static const int FLOATS_PER_PIXEL = sizeof(PixelData32) / FLOAT_SIZE; // 4
	static const int PIXEL_PER_BLOCK = sizeof(PixelData32T8) / FLOATS_PER_PIXEL / FLOAT_SIZE; // 8

	float* buffer = new float[FLOAT_SIZE * (2 * PIXEL_PER_BLOCK + (simulatedWidth-width) * FLOATS_PER_PIXEL)];
	int rgbDataOffsetFloat = 0;
	int dataOffsetFloat = 0;
	int lineOffsetPixel = 0;
	int bufferOffsetFloat;
	int simulatedDataSize = simulatedWidth * simulatedHeight * FLOATS_PER_PIXEL;
	int dataSize = width * height * FLOATS_PER_PIXEL;

	while (dataOffsetFloat < simulatedDataSize)
	{
		int pixelsToProcess = width - lineOffsetPixel;
		lineOffsetPixel = 0;
		int lineRem = pixelsToProcess % PIXEL_PER_BLOCK;
		
		pixelsToProcess -= lineRem;
		transposeFloatAVX(rgbaData + rgbDataOffsetFloat, (float*)data + dataOffsetFloat, pixelsToProcess);
		rgbDataOffsetFloat += pixelsToProcess * FLOATS_PER_PIXEL;
		dataOffsetFloat += pixelsToProcess * FLOATS_PER_PIXEL;
		memcpy(buffer, rgbaData + rgbDataOffsetFloat, lineRem * FLOATS_PER_PIXEL * FLOAT_SIZE);
		rgbDataOffsetFloat += lineRem * FLOATS_PER_PIXEL;
		bufferOffsetFloat = lineRem * FLOATS_PER_PIXEL;

		for (int i = 0; i < (simulatedWidth - width); i++)
		{
			memcpy(buffer + bufferOffsetFloat, rgbaData + rgbDataOffsetFloat - FLOATS_PER_PIXEL, FLOATS_PER_PIXEL * FLOAT_SIZE);
			bufferOffsetFloat += 4;
		}
		
		int bufferAlignRem = bufferOffsetFloat % (PIXEL_PER_BLOCK * FLOATS_PER_PIXEL);
		
		if (bufferAlignRem != 0)
		{
			int floatsToCopy = PIXEL_PER_BLOCK * FLOATS_PER_PIXEL - bufferAlignRem;
		
			if (rgbDataOffsetFloat < dataSize) {
				memcpy(buffer + bufferOffsetFloat, rgbaData + rgbDataOffsetFloat, floatsToCopy * FLOAT_SIZE);
			} 
			else
			{
				memcpy(buffer + bufferOffsetFloat, rgbaData + rgbDataOffsetFloat - width * FLOATS_PER_PIXEL, floatsToCopy * FLOAT_SIZE);
			}
			rgbDataOffsetFloat += floatsToCopy;

			bufferOffsetFloat += floatsToCopy;
			lineOffsetPixel = floatsToCopy / FLOATS_PER_PIXEL;
		}
		
		transposeFloatAVX(buffer, (float*)data + dataOffsetFloat, bufferOffsetFloat / FLOATS_PER_PIXEL);
		dataOffsetFloat += bufferOffsetFloat;

		if (rgbDataOffsetFloat >= dataSize) {
			rgbDataOffsetFloat -= width * FLOATS_PER_PIXEL;
		}
	}


	//int simulatedLineSize = simulatedWidth * FLOATS_PER_PIXEL;
	//int heightRem = simulatedHeight - height;
	//for (int i = 0; i < heightRem; i++)
	//{
	//	memcpy((float*)data + simulatedDataSize + i * simulatedLineSize, (float*)data + simulatedDataSize - simulatedLineSize, simulatedLineSize * FLOAT_SIZE);
	//}

	delete[] buffer;
}

ImageDataPtr Image::getRawPixelData()
{
	ImageDataPtr imageData = std::make_shared<std::vector<float>>(slots * 32);
	transposeFloatAVX_reverse((float*)data, &imageData->operator[](0), simulatedWidth*simulatedHeight);
	return imageData;
}

void Image::SetPixel(uint x, uint y, PixelData32 color)
{
	PixelPos pos = GetPixelPos(x, y);

	data[pos.slot].red[pos.index] = color.R;
	data[pos.slot].green[pos.index] = color.G;
	data[pos.slot].blue[pos.index] = color.B;
	data[pos.slot].alpha[pos.index] = color.A;
}

PixelData32 Image::GetPixel(uint x, uint y)
{
	PixelPos pos = GetPixelPos(x, y);

	return PixelData32(
		data[pos.slot].red[pos.index], 
		data[pos.slot].green[pos.index],
		data[pos.slot].blue[pos.index], 
		data[pos.slot].alpha[pos.index]);
}

inline PixelPos Image::GetPixelPos(uint x, uint y)
{
	div_t tmp = div(y*width + x, 8);
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