// JPEG-Encoder-SIMD.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

#include "Image.h"
#include "ImageLoader.h"

int main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " NAME" << std::endl;
		return 1;
	}

	std::string file(argv[1]);

	ImagePtr image = ImageLoader::FromPPM(file);
	image->convertToYCbCr();
	image->applySepia();

	ImageLoader::SaveToPPM("C:\\Users\\Markus\\Desktop\\Test2.ppm", image);

	return 0;
}

