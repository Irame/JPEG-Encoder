#pragma once
#include "Pixel.h"
#include <string>
#include "ColorCoding.h"
class Image
{
public:
	Image();
	~Image();
	
	void getData(Pixel** data);
	ColorCoding getColorCoding();
	void setStep(unsigned int stepX, unsigned int stepY);
	void readPPM(std::string path);
private:
	unsigned int stepX;
	unsigned int stepY;
	unsigned int width;
	unsigned int heigth;
	ColorCoding colorCoding;
	Pixel** data;
};

