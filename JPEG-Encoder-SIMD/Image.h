#pragma once
#include <memory>
#include <assert.h>
#include "ColorNames.h"
#include "SamplingScheme.h"
#include <vector>
#include "HuffmanCoding.h"
#include "QuantizationTables.h"

class Image;

typedef unsigned int uint;
typedef std::shared_ptr<Image> ImagePtr;

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

	float* getChannel(ColorChannelName channel) { return *(reinterpret_cast<float**>(this) + channel); }
};

class Image
{
	void setRawPixelDataDirect(float* rgbaData);

	size_t getPixelPos(ColorChannelName channelIdx, size_t x, size_t y) const;

protected:
	std::unique_ptr<ImageData> channels;

	const Dimension2D imageSize;
	const Dimension2D stepSize;
	const Dimension2D simulatedSize;

	size_t blocksPerChannel[3];
	Dimension2D channelSizes[3];

	const SamplingScheme samplingScheme;
	const QTable luminance;
	const QTable chrominance;

public:
	Image(size_t width, size_t height, SamplingScheme scheme, QTable luminance, QTable chrominance);
	Image(Image& origImage);

	const Dimension2D& getImageSize() const;
	const Dimension2D& getSimulatedSize() const;

	void setRawPixelData(float* rgbaData);

	std::vector<float> getRawPixelDataSimulated();
	std::vector<float> getRawPixelData();

	void setPixel(size_t x, size_t y, const PixelData32& color);
	void getPixel(PixelData32& ref, size_t x, size_t y) const;

	void applySepia();

	const SamplingScheme& getSamplingScheme() const  { return samplingScheme; }
	const QTable& getLuminanceQTable() const { return luminance; }
	const QTable& getChrominanceQTable() const { return chrominance; }
};