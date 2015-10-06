#pragma once
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
	Image(int width, int height, ColorCoding coding);
	~Image();
	
	const ColorChannel& getColorChannel(ColorName colorName) const;
	ColorCoding getColorCoding() const;
	void setStep(unsigned int stepX, unsigned int stepY);
	void switchColorCoding(ColorCoding newCoding);
	void scaleColor(ColorName colorName, float factor);
private:
	unsigned int stepX;
	unsigned int stepY;
	unsigned int width;
	unsigned int height;
	ColorCoding colorCoding;
	ColorChannel* channel[3];

	template<ColorCoding, ColorCoding>
	void switchColorCoding();
};
