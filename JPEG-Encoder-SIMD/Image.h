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

struct Dimension2D
{
	size_t width;
	size_t height;

	Dimension2D(size_t width, size_t height)
		: width(width), height(height)
	{}

	bool operator==(const Dimension2D& other) const
	{
		return other.width == width && other.height == height;
	}
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
	ImageData(Dimension2D size) {
		size_t dataSizeInBytes = size.width * size.height * sizeof(float);
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

	const Dimension2D imageSize;
	const Dimension2D stepSize;
	const Dimension2D simulatedSize;

	int blocksPerChannel[3];
	Dimension2D channelSizes[3];

	const SamplingScheme samplingScheme;

	void setRawPixelDataDirect(float* rgbaData);

public:
	Image(size_t width, size_t height, SamplingScheme scheme);

	void setRawPixelData(float* rgbaData);
	std::vector<float> getRawPixelDataSimulated();
	std::vector<float> getRawPixelData();
	void SetPixel(uint x, uint y, const PixelData32& color);
	void GetPixel(PixelData32& ref, uint x, uint y) const;
	size_t getPizelPos(int channelIdx, uint x, uint y) const;
	void convertToYCbCrAVX();
	void convertToRGBAVX();
	void applySepiaAVX();
	void multiplyColorChannelByAVX(int colorChannel, float val);
	void reduceWidthResolutionColorChannel(int channel, int factor, ReductionMethod method);
	void reduceHeightResolutionColorChannel(int channelIdx, int factor, ReductionMethod method);
	void reduceResolutionBySchema();
	const Dimension2D& getImageSize() const;
	const Dimension2D& getSimulatedSize() const;
};