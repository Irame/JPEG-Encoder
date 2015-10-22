#include "stdafx.h"
#include "ImageCC.h"

ImageCC::ImageCC(int width, int height) 
	: width(width), height(height), stepX(8), stepY(8), 
	simulatedWidth(width + (width % stepX == 0 ? 0 : stepX - width % stepX)), 
	simulatedHeight(height + (height % stepY == 0 ? 0 : stepY - height % stepY))
{
}

void ImageCC::setChannel(int channelIndex, std::vector<ColorBlock> chan, int width, int height)
{
	channels[channelIndex] = chan;
	channelWidthRatio[channelIndex] = width / this->width;
	channelHeightRatio[channelIndex] = height / this->height;
}