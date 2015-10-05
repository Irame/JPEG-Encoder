#pragma once
enum ColorCoding {
	RGB, YCbCr
};


inline std::ostream& operator<<(std::ostream& strm, const ColorCoding& colorCoding)
{
	switch (colorCoding)
	{
		case RGB: strm << "RGB"; break;
		case YCbCr: strm << "YCbCr"; break;
	}
	return strm;
}
