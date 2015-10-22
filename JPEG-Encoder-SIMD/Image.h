#pragma once

#include <vector>
#include <memory>
#include "SamplingScheme.h"

typedef unsigned int uint;
typedef std::shared_ptr<std::vector<float>> ImageDataPtr;

struct PixelPos {
	int slot;
	int index;

	PixelPos(int slot, int index) : slot(slot), index(index) {};
};

struct PixelData32 {
	float R, G, B, A;

	PixelData32() : A(1.0) {}

	PixelData32(float r, float g, float b, float a = 1.0)
		: R(r), G(g), B(b), A(a)
	{}
};

struct PixelData32T8 {
	float red[8];
	float green[8];
	float blue[8];
	float alpha[8];
};

class Image
{
private:
	PixelData32T8* data;
	const size_t width, height;
	const uint stepX, stepY;
	const size_t simulatedWidth, simulatedHeight;
	const uint slots;
public:
	Image(size_t width, size_t height, SamplingScheme samplingScheme);
	~Image();

	size_t getWidth() { return width; }
	size_t getHeight() { return height; }
	size_t getSimulatedWidth() { return simulatedWidth; }
	size_t getSimulatedHeight() { return simulatedHeight; }

	void setRawPixelData(float* rgbaData);
	std::vector<float> getRawPixelDataSimulated();
	std::vector<float> getRawPixelData();

	void SetPixel(uint x, uint y, PixelData32 color);
	void GetPixel(PixelData32& ref, uint x, uint y);

	void convertToYCbCr();
	void convertToYCbCrAVX();

	void convertToRGB();
	void convertToRGBAVX();

	void applySepia();
	void applySepiaAVX();
	void multiplyColorChannelByAVX(int colorChannel, float val);
private:
	inline PixelPos GetPixelPos(uint x, uint y);
	void setRawPixelDataDirect(float* rgbaData);
};

typedef std::shared_ptr<Image> ImagePtr;