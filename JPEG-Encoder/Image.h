#pragma once
#include "Pixel.h"
#include <string>
#include "ColorCoding.h"
#include "ColorChannel.h"
#include <memory>

class Image;
typedef std::shared_ptr<Image> ImagePtr;

class Image
{
public:
	static ImagePtr readPPM(std::string path);

	Image();
	Image(int width, int height);
	~Image();
	
	const ColorChannel& getColorChannel(ColorName colorName) const;
	ColorCoding getColorCoding();
	void setStep(unsigned int stepX, unsigned int stepY);
private:
	unsigned int stepX;
	unsigned int stepY;
	unsigned int width;
	unsigned int heigth;
	ColorCoding colorCoding;
	ColorChannel channel[3];
};

