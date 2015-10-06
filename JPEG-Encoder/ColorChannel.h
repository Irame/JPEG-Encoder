#pragma once
#include "ColorNames.h"
#include <memory>

class ColorChannel;
typedef std::shared_ptr<ColorChannel> ColorChannelPtr;

class ColorChannel
{
public:
	ColorChannel();
	ColorChannel(ColorName colorName, int width, int height);
	~ColorChannel();

	void setColorValue(float value, int x, int y);

	float* operator[](int idx) const;
	int getWidth() const;
	int getHeight() const;
	ColorName getColorName() const;
	void scale(float factor);
private:
	ColorName colorName;
	
	int width;
	int height;
	
	float** data;
};
