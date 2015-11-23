// JPEG-Encoder-SIMD.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Benchmark.h"
#include <vector>
#include <string>
#include "BitBuffer.h"
#include "JPEGSegments.h"
#include "Image.h"
#include "ImageLoader.h"
#include "PointerMatrix.h"
#include "DCT.h"

using namespace std;

byte testValues[10000000];

template <typename TElem>
ostream& operator<<(ostream& os, const vector<TElem>& vec) {
	typedef typename vector<TElem>::const_iterator iter_t;
	const iter_t iter_begin = vec.begin();
	const iter_t iter_end = vec.end();
	os << "[";
	for (iter_t iter = iter_begin; iter != iter_end; ++iter) {
		cout << ((iter != iter_begin) ? ", " : "") << *iter;
	}
	os << "]";
	return os;
}

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

	Serialize(startOfImage, bitBuffer);
	Serialize(app0, bitBuffer);
	Serialize(startOfFrame0, bitBuffer);
	Serialize(endOfImage, bitBuffer);

	//for (int i = 0; i < 10000000; i++)
	//{
	//	bitBuffer.pushBit(testValues[i] == 0);
	//}

	bitBuffer.writeToFile(filePath);

	//cout << bitBuffer << endl;
}

void testDCT()
{
	float rowOne[]		=	{ 140, 144, 147, 140, 140, 155, 179, 175 };
	float rowTwo[]		=	{ 144, 152, 140, 147, 140, 148, 167, 179 };
	float rowThree[]	=	{ 152, 155, 136, 167, 163, 162, 152, 172 };
	float rowFour[]		=	{ 168, 145, 156, 160, 152, 155, 136, 160 };
	float rowFive[]		=	{ 162, 148, 156, 148, 140, 136, 147, 162 };
	float rowSix[]		=	{ 147, 167, 140, 155, 155, 140, 136, 162 };
	float rowSeven[]	=	{ 136, 156, 123, 167, 162, 144, 140, 147 };
	float rowEight[]	=	{ 148, 155, 136, 155, 152, 147, 147, 136 };

	float arr[64];

	for (int i = 0; i < 64; i++)
	{
		arr[i] = i+1;
	}

	float resultBytes[64];
	PointerMatrix result(resultBytes);
	mat8x8 matResult;

	//PointerMatrix testMatrix = PointerMatrix(arr);
	PointerMatrix testMatrix = PointerMatrix(rowOne, rowTwo, rowThree, rowFour, rowFive, rowSix, rowSeven, rowEight);
	auto kokMatrix = mat8x8(rowOne, rowTwo, rowThree, rowFour, rowFive, rowSix, rowSeven, rowEight);
	
	//DCT::directDCT(testMatrix);
	DCT::seperateDCT(testMatrix, result);
	//DCT::dct_ii(testMatrix);

	size_t runs = 1000;
	//benchmark("Direct DCT", runs, [&testMatrix, &result]() {
	//	for (int i = 0; i < 1024; i++) {
	//		DCT::directDCT(testMatrix, result);
	//	}
	//});
	//benchmark("Seperate DCT", runs, [&testMatrix, &result]() {
	//	for (int i = 0; i < 1024; i++) {
	//		DCT::seperateDCT(testMatrix, result);
	//	}
	//});
	//benchmark("Kok DCT", runs, [&kokMatrix]() {
	//	for (int i = 0; i < 1024; i++) {
	//		mat8x8 result = DCT::kokDCT(kokMatrix);
	//	}
	//});
	//benchmark("Kok Simple DCT", runs, [&kokMatrix]() {
	//	for (int i = 0; i < 1024; i++) {
	//		mat8x8 result = DCT::kokSimple(kokMatrix);
	//	}
	//});
	benchmark("Arai DCT", runs, [&kokMatrix, &matResult]() {
		for (int i = 0; i < 1024; i++) {
			matResult = DCT::araiDCT(kokMatrix);
		}
	});
	cout << endl;
	for (size_t i = 0; i < 8; i++)
	{
		for (size_t j = 0; j < 8; j++)
		{
			//cout << round(result[i][j]) << " | ";
			printf("%5.0f | ", roundf(matResult.at(i, j)));
		}
		cout << endl;
	}
	cout << "end of dct" << endl;


	benchmark("Arai DCT AVX", runs, [&kokMatrix, &matResult]() {
		for (int i = 0; i < 1024; i++) {
			matResult = DCT::araiDCTAVX(kokMatrix);
		}
	});

	cout << endl;
	for (size_t i = 0; i < 8; i++)
	{
		for (size_t j = 0; j < 8; j++)
		{
			//cout << round(result[i][j]) << " | ";
			printf("%5.0f | ", roundf(matResult.at(i, j)));
		}
		cout << endl;
	}
	cout << "end of dct" << endl;

	//cout << endl;
	//for (int i = 0; i < 8; i++)
	//{
	//	for (int j = 0; j < 8; j++)
	//	{
	//		printf("%5.0f | ", round(result[i][j]));
	//	}
	//	cout << endl;
	//}
	//cout << "end of dct" << endl;
}

void testHuffmanEncoding()
{
	vector<byte> allSymbols{ 0, 1, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6 };
	cout << "All symbols: " << allSymbols << endl;
	
	auto huffmanTable = HuffmanTable<byte>::create(16, allSymbols);
	cout << "Huffman Table:" << endl << *huffmanTable;

	vector<byte> encodeDecodeTestSymbols{ 3, 0, 4 };
	cout << "Test symbols: " << encodeDecodeTestSymbols << endl;

	auto encodedSymbols = huffmanTable->encode(encodeDecodeTestSymbols);
	cout << "Encoded symbols: " << *encodedSymbols << endl;

	vector<byte> decodedSymbols = huffmanTable->decode(encodedSymbols);

	cout << "Decoded symbols: " << decodedSymbols << endl;
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

	//benchmark("testHuffmanEncoding", 1, [&]() {
	//	testHuffmanEncoding();
	//});
	//return  0;

	/*if (argc < 3) {
		cerr << "Usage: " << argv[0] << " <Source File> <Destination File>" << endl;
		return 1;
	}

	string srcFile(argv[1]);
	string dstFile(argv[2]);

	SamplingScheme scheme = SamplingScheme::Scheme422;

	cout << "Load image file: " << srcFile << endl;
	ImageCCPtr image = nullptr;
	

	benchmark("ImageLoader::Load()",1, [&]() {
		image = ImageLoader::Load(srcFile, scheme);
	});*/



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


	//cout << "Reduce channel resolution for scheme." << endl;
	//benchmark("reduceResolutionBySchema", 1, [&]() {
	//	image->reduceResolutionBySchema();
	//});


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

	/*cout << "Save image file: " << dstFile << endl;
	benchmark("ImageLoader::Save()", 1, [&]() {
		ImageLoader::Save(dstFile, image);
	});*/

	std::cout << "Test DCT" << endl;
	benchmark("Test DCT", 1, [&]()
	{
		testDCT();
	});

	return 0;
}