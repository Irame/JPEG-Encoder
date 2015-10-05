#pragma once
enum RGBColorName
{
	InvalidRgb = -1, R = 0, G = 1, B = 2
};

enum YCbCrColorName
{
	InvalidYCbCr = -1, Y = 0, Cb = 1, Cr = 2
};

union ColorName
{
	ColorName()
	{
		rgbColorName = InvalidRgb;
	}

	ColorName(int i)
	{
		rgbColorName = static_cast<RGBColorName>(i);
	}

	RGBColorName rgbColorName;
	YCbCrColorName yCbCrColorName;
};