#pragma once
class Pixel
{
private:
	float data[3];

public:
	Pixel();
	Pixel(float v1, float v2, float v3);
	~Pixel();

	enum RGBColorName
	{
		R = 0, G = 1, B = 2
	};

	enum YCbCrColorName
	{
		Y = 0, Cb = 1, Cr = 2
	};

	inline float getColorValue(int idx) const
	{
		return data[idx];
	}

	inline void setColorValue(int idx, float value)
	{
		data[idx] = value;
	}
};

