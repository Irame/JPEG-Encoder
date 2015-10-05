#pragma once
#include "ColorNames.h"

class ColorChannel
{
public:
	ColorChannel();
	ColorChannel(ColorName colorName, int width, int height);
	~ColorChannel();

	void setColorValue(float value, int x, int y);

private:
	ColorName colorName;
	
	int width;
	int height;
	
	float** data;
};

