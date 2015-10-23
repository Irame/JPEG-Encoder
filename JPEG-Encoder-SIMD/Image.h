#pragma once
#include <vector>
#include <memory>
#include <cassert>
#include "SamplingScheme.h"

class Image;

typedef unsigned int uint;
typedef std::shared_ptr<Image> ImageCCPtr;

struct PixelData32 {
	float R, G, B, A;

	PixelData32() : R(0.0f), G(0.0f), B(0.0f), A(1.0f) {}

	PixelData32(float r, float g, float b, float a = 1.0f)
		: R(r), G(g), B(b), A(a)
	{}
};

struct ColorBlock
{
	float colVal[8];
};

struct ImageData
{
private:
	ColorBlock *r;
	ColorBlock *g;
	ColorBlock *b;

public:
	ImageData(size_t width, size_t height) {
		size_t dataSizeInBytes = width * height * sizeof(float);
		assert(dataSizeInBytes % (8 * sizeof(float)) == 0);

		r = static_cast<ColorBlock*>(_mm_malloc(dataSizeInBytes, 32));
		g = static_cast<ColorBlock*>(_mm_malloc(dataSizeInBytes, 32));
		b = static_cast<ColorBlock*>(_mm_malloc(dataSizeInBytes, 32));
	}

	~ImageData()
	{
		_mm_free(r);
		_mm_free(g);
		_mm_free(b);
	}

	float* red(size_t offset = 0) { return reinterpret_cast<float*>(r) + offset; }

	float* green(size_t offset = 0) { return reinterpret_cast<float*>(g) + offset; }

	float* blue(size_t offset = 0) { return reinterpret_cast<float*>(b) + offset; }

	float* getChannel(int channel) { return *(reinterpret_cast<float**>(this) + channel); }
};

class Image
{
	std::unique_ptr<ImageData> channels;

	float channelWidthRatio[3];
	float channelHeightRatio[3];

	const size_t width, height;
	const size_t stepX, stepY;
	const size_t simulatedWidth, simulatedHeight;

	int blocksPerChannel[3];

	const SamplingScheme samplingScheme;

	void setRawPixelDataDirect(float* rgbaData);

public:
	Image(size_t width, size_t height, SamplingScheme scheme);

	void setRawPixelData(float* rgbaData);
	std::vector<float> getRawPixelDataSimulated();
	std::vector<float> getRawPixelData();
	void SetPixel(uint x, uint y, const PixelData32& color);
	void GetPixel(PixelData32& ref, uint x, uint y) const;
	void convertToYCbCrAVX();
	void convertToRGBAVX();
	void applySepiaAVX();
	void multiplyColorChannelByAVX(int colorChannel, float val);
	void reduceWidthResolutionColorChannel(int channel, int factor, ReductionMethod method);
	void reduceHeightResolutionColorChannel(int channelIdx, int factor, ReductionMethod method);
	size_t getWidth() const;
	size_t getHeight() const;
	size_t getSimulatedWidth() const;
	size_t getSimulatedHeight() const;
};