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
	

	benchmark("ImageLoader::Load()",1, [&]() {
		image = ImageLoader::Load(srcFile, 16, 16);
	});


	//std::cout << "Convert image to YCbCr." << std::endl;
	//benchmark("convertToYCbCr",1, [&]() {
	//	image->convertToYCbCr();
	//});
	
	//std::cout << "Aplying Sepia Filter." << std::endl;
	//benchmark("applySepiaAVX",1, [&]() {
	//	image->applySepiaAVX();
	//});

	std::cout << "Convert image to YCbCr AVX." << std::endl;
	benchmark("convertToYCbCrAVX",1, [&]() {
		image->convertToYCbCrAVX();
	});

	/*std::cout << "Cancle out Cb and Cr Channel." << std::endl;
	benchmark("multiplyColorChannelByAVX",1, [&]() {
		image->multiplyColorChannelByAVX(1, 0.75);
		image->multiplyColorChannelByAVX(2, 0.5);
	});*/

	std::cout << "Convert image to RGB AVX." << std::endl;
	benchmark("convertToRGBAVX",1, [&]() {
		image->convertToRGBAVX();
	});

	//std::cout << "Convert image to RGB." << std::endl;
	//benchmark("convertToRGB",1, [&]() {
	//	image->convertToRGB();
	//});

	//std::cout << "Apply Sebia filter" << std::endl;
	//benchmark("applySepia",1, [&]() {
	//	image->applySepia();
	//});

	std::cout << "Save image file: " << dstFile << std::endl;
	benchmark("ImageLoader::Save()", 1, [&]() {
		ImageLoader::Save(dstFile, image);
	});

	return 0;
}

