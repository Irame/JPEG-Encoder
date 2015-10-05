#pragma once
#include "ColorNames.h"

class ColorChannel
{
public:
	ColorChannel();
	ColorChannel(ColorName colorName, int width, int height);
	~ColorChannel();

	void setColorValue(float value, int x, int y);

	float* operator[](int idx) const;
private:
	ColorName colorName;
	
	int width;
	int height;
	
	float** data;
};

