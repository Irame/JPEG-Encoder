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
		image = ImageLoader::Load(srcFile);
	});


	//std::cout << "Convert image to YCbCr." << std::endl;
	//benchmark(1, [&]() {
	//	image->convertToYCbCr();
	//});
	
	std::cout << "Convert image to YCbCr AVX." << std::endl;
	benchmark(1, [&]() {
		image->convertToYCbCrAVX();
	});

	std::cout << "Convert image to YCbCr AVX." << std::endl;
	benchmark(1, [&]() {
		image->convertToRGBAVX();
	});

	//std::cout << "Convert image to RGB." << std::endl;
	//benchmark(1, [&]() {
	//	image->convertToRGB();
	//});

	//std::cout << "Apply Sebia filter" << std::endl;
	//benchmark(1, [&]() {
	//	image->applySepia();
	//});

	std::cout << "Save image file: " << dstFile << std::endl;
	benchmark(1, [&]() {
		ImageLoader::Save(dstFile, image);
	});

	return 0;
}

