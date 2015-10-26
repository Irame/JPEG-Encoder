#pragma once

enum RGBColorName
{
	R = 0, G = 1, B = 2
};

enum YCbCrColorName
{
	Y = 0, Cb = 1, Cr = 2
};

union ColorChannelName
{
	ColorChannelName()
	{
		index = 0;
	}

	ColorChannelName(int i)
	{
		index = i;
	}

	operator int() const 
	{
		return index;
	}

	int index;
	RGBColorName rgbColorName;
	YCbCrColorName yCbCrColorName;
};

