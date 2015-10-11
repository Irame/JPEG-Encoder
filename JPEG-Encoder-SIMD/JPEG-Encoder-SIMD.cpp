// JPEG-Encoder-SIMD.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

#include "Benchmark.h"

#include "Image.h"
#include "ImageLoader.h"

int main(int argc, char* argv[])
{
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <Source File> <Destination File>" << std::endl;
		return 1;
	}

	std::string srcFile(argv[1]);
	std::string dstFile(argv[2]);

	std::cout << "Load image file: " << srcFile << std::endl;
	ImagePtr image = nullptr;
	
	benchmark(1, [&]() {
		image = ImageLoader::FromPPM(srcFile);
	});

	std::cout << "Convert image to YCbCr." << std::endl;
	benchmark(100, [&](){
		image->convertToYCbCr();
	});

	//image->convertToYCbCr();
	image->applySepia();

	ImageLoader::SaveToPPM(dstFile, image);

	return 0;
}

