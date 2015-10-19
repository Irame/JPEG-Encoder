#pragma once
#include <vector>

struct ColorBlock
{
	float colVal[8];
};

class ImageCC
{
	std::vector<ColorBlock> channel[4];

	float channelWidthRatio[4];
	float channelHeightRatio[4];

	const int width, height;
	const int stepX, stepY;
	const int simulatedWidth, simulatedHeight;

public:
	ImageCC(int width, int height, int stepX, int stepY);

	void setChannel(int channelIndex, std::vector<ColorBlock> chan, int width, int height);
};

