#pragma once

#include <vector>
#include <memory>

typedef unsigned int uint;

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
	uint slots;
public:
	Image();
	Image(size_t width, size_t height);
	~Image();

	size_t getWidth() { return width; }
	size_t getHeight() { return height; }

	void        setRawPixelData(float* rgbaData);
	void        getRawPixelData(float* rgbaDataDest);

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
