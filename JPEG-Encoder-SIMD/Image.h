#pragma once

#include <vector>
#include <memory>

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
	size_t width, height;
	size_t simulatedWidth, simulatedHeight;
	uint slots;
	uint stepX, stepY;
public:
	Image(size_t width, size_t height, size_t stepX, size_t stepY);
	~Image();

	size_t getWidth() { return width; }
	size_t getHeight() { return height; }
	size_t getSimulatedWidth() { return simulatedWidth; }
	size_t getSimulatedHeight() { return simulatedHeight; }

	void        setRawPixelData(float* rgbaData);
	ImageDataPtr getRawPixelData();
	void setRawPixelData2(float* rgbaData);

	void        SetPixel(uint x, uint y, PixelData32 color);
	PixelData32 GetPixel(uint x, uint y);

	void applySepia();
	void convertToYCbCr();
	void convertToYCbCrAVX();

	void convertToRGB();
	void convertToRGBAVX();
	void applySepiaAVX();
private:
	inline PixelPos GetPixelPos(uint x, uint y);
};

typedef std::shared_ptr<Image> ImagePtr;