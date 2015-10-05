#pragma once
#include <iostream>

enum RGBColorName
{
	InvalidRgb = -1, R = 0, G = 1, B = 2
};

inline std::ostream& operator<<(std::ostream& strm, const RGBColorName& colorCoding)
{
	switch (colorCoding)
	{
	case InvalidRgb: strm << "InvalidRgb";
	case R: strm << "R"; break;
	case G: strm << "G"; break;
	case B: strm << "B"; break;
	}
	return strm;
}

enum YCbCrColorName
{
	InvalidYCbCr = -1, Y = 0, Cb = 1, Cr = 2
};

inline std::ostream& operator<<(std::ostream& strm, const YCbCrColorName& colorCoding)
{
	switch (colorCoding)
	{
	case InvalidYCbCr: strm << "InvalidYCbCr";
	case Y: strm << "Y"; break;
	case Cb: strm << "Cb"; break;
	case Cr: strm << "Cr"; break;
	}
	return strm;
}

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