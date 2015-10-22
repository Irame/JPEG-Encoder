#pragma once
#include <vector>
#include <memory>

struct ColorBlock
{
	float colVal[8];
};

class ImageCC
{
	std::vector<ColorBlock> channels[3];

	float channelWidthRatio[3];
	float channelHeightRatio[3];

	const int width, height;
	const int stepX, stepY;
	const int simulatedWidth, simulatedHeight;

public:
	ImageCC(int width, int height);

	void setChannel(int channelIndex, std::vector<ColorBlock> chan, int width, int height);
};

typedef std::shared_ptr<ImageCC> ImageCCPtr;