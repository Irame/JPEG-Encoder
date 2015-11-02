// JPEG-Encoder-SIMD.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

#include "Benchmark.h"

#include "ImageLoader.h"
#include "BitBuffer.h"
#include "JPEGSegments.h"
#include <map>
#include "HuffmanCoding.h"

byte testValues[10000000];

void bitBufferTest(string filePath)
{
	BitBuffer bitBuffer;
	
	//byte test[2]{ 0xff, 0x55 };

	//byte testOut[1];

	//bitBuffer.pushBit(false);
	//bitBuffer.pushBit(false);
	//bitBuffer.pushBit(false);
	//bitBuffer.pushBit(false);
	//bitBuffer.pushBit(false);
	//bitBuffer.pushBit(false);
	//bitBuffer.pushBits(10, test, 4);

	//cout << bitBuffer << endl;

	//bitBuffer.getBits(5, testOut, 8);

	//cout << hex << int(testOut[0]) << endl;

	//for (int i = 0; i < bitBuffer.getSize(); i++) {
	//	cout << bitBuffer.getBit(i);
	//}
	//cout << endl;

	//bitBuffer.pushBits(12, test);
	//cout << bitBuffer << endl;
	//bitBuffer.pushBits(14, test);

	//bitBuffer.pushBit(true);
	//bitBuffer.pushBit(true);
	//bitBuffer.pushBit(false);
	//bitBuffer.pushBit(true);
	//bitBuffer.pushBit(false);
	//bitBuffer.pushBit(false);
	//bitBuffer.pushBit(true);

	JPEGSegments::StartOfImage startOfImage;
	JPEGSegments::APP0 app0;
	JPEGSegments::StartOfFrame0 startOfFrame0(1680,900, SamplingScheme::Scheme422);

	JPEGSegments::EndOfImage endOfImage;

	JPEGSegments::Serialize(startOfImage, bitBuffer);
	JPEGSegments::Serialize(app0, bitBuffer);
	JPEGSegments::Serialize(startOfFrame0, bitBuffer);
	JPEGSegments::Serialize(endOfImage, bitBuffer);

	//for (int i = 0; i < 10000000; i++)
	//{
	//	bitBuffer.pushBit(testValues[i] == 0);
	//}

	bitBuffer.writeToFile(filePath);

	//cout << bitBuffer << endl;
}

void testHuffmanEncoding()
{
	auto result = HuffmanCoding::createHuffmanTable(std::vector<byte> {0, 0, 0, 2, 2, 3, 4});
	for (auto it = result.cbegin(); it != result.cend(); ++it)
	{
		std::cout << it->first << ": " << *it->second << endl;
	}
}

int main(int argc, char* argv[])
{
	//for (int i = 0; i < 10000000; i++)
	//{
	//	testValues[i] = rand() % 2;
	//}

	//benchmark("bitBufferTest", 100, [&]() {
	//	bitBufferTest(argv[2]);
	//});

	//benchmark("testHuffmanEncoding", 100, [&]() {
	//	testHuffmanEncoding();
	//});
	//return  0;

	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <Source File> <Destination File>" << std::endl;
		return 1;
	}

	std::string srcFile(argv[1]);
	std::string dstFile(argv[2]);

	SamplingScheme scheme = SamplingScheme::Scheme422;

	std::cout << "Load image file: " << srcFile << std::endl;
	ImageCCPtr image = nullptr;
	

	benchmark("ImageLoader::Load()",1, [&]() {
		image = ImageLoader::Load(srcFile, scheme);
	});


	//std::cout << "Convert image to YCbCr." << std::endl;
	//benchmark("convertToYCbCr",1, [&]() {
	//	image->convertToYCbCr();
	//});
	
	//std::cout << "Aplying Sepia Filter." << std::endl;
	//benchmark("applySepia",1, [&]() {
	//	image->applySepia();
	//});

	//std::cout << "Convert image to YCbCr AVX." << std::endl;
	//benchmark("convertToYCbCr",1, [&]() {
	//	image->convertToYCbCr();
	//});


	std::cout << "Reduce channel resolution for scheme." << std::endl;
	benchmark("reduceResolutionBySchema", 1, [&]() {
		image->reduceResolutionBySchema();
	});


	//std::cout << "Cancle out Cb and Cr Channel." << std::endl;
	//benchmark("multiplyColorChannelBy",1, [&]() {
	//	image->multiplyColorChannelBy(0, 0);
	//	image->multiplyColorChannelBy(1, 0);
	//});

	//std::cout << "Convert image to RGB AVX." << std::endl;
	//benchmark("convertToRGB",1, [&]() {
	//	image->convertToRGB();
	//});

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

