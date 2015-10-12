#include "stdafx.h"
#include "Image.h"

#include "SIMD.h"

Image::Image()
{
}

Image::Image(size_t width, size_t height)
	: data(nullptr), width(width), height(height), slots(0)
{
	slots = static_cast<uint>(ceil((width*height) / 8.0));
	data = new PixelData32T8[slots];
}

Image::~Image()
{
	delete[] data;
}

void Image::setRawPixelData(float* rgbaData)
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

void Image::getRawPixelData(float* rgbaDataDest)
{
	transposeFloatAVX_reverse((float*)data, rgbaDataDest, width*height);
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


