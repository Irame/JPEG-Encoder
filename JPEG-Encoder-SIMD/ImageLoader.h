#pragma once

#include <string>

#include "Image.h"

class ImageLoader
{
public:
	ImageLoader();
	~ImageLoader();

	static ImagePtr FromPPM(std::string path);
	static ImagePtr FromPPM2(std::string path);


	static void SaveToPPM(std::string path, ImagePtr image);


private:
	static inline int clamp(int lower, int x, int upper);
};

