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
#include "QuantizationTables.h"

using namespace std;

#if MSVC
#define COMPILER_PREFACE std::cout << "Built with msvc " << _MSC_VER << std::endl
#endif
#if GCC
#define COMPILER_PREFACE std::cout << "Built with g++ " << __VERSION__ << std::endl
#endif
#if CLANG
#define COMPILER_PREFACE std::cout << "Built with clang++ " << __clang_version__ << std::endl
#endif
#if INTEL
#define COMPILER_PREFACE std::cout << "Built with the intel compiler " << __ICL << std::endl
#endif

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
	JPEGSegments::StartOfFrame0 startOfFrame0(1680,900, Sampling::Scheme422);

	vector<byte> allSymbols{ 0, 1, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6 };
	cout << "All symbols: " << allSymbols << endl;

	auto huffmanTable = HuffmanTable<byte>::create(16, allSymbols);
	cout << "Huffman Table:" << endl << *huffmanTable;

	JPEGSegments::DefineHuffmannTable defineHuffmanTable(3, JPEGSegments::HuffmanTableType::AC, *huffmanTable);

	JPEGSegments::DefineQuantizationTable quantizationTableLuminance(YCbCrColorName::Y, JPEGQuantization::luminance);
	JPEGSegments::DefineQuantizationTable quantizationTableChrominance(YCbCrColorName::Cb, JPEGQuantization::chrominance);

	JPEGSegments::EndOfImage endOfImage;

	Serialize(startOfImage, bitBuffer);
	Serialize(app0, bitBuffer);
	Serialize(startOfFrame0, bitBuffer);
	Serialize(defineHuffmanTable, bitBuffer);
	Serialize(quantizationTableLuminance, bitBuffer);
	Serialize(quantizationTableChrominance, bitBuffer);
	Serialize(endOfImage, bitBuffer);

	//for (int i = 0; i < 10000000; i++)
	//{
	//	bitBuffer.pushBit(testValues[i] == 0);
	//}

	bitBuffer.writeToFile(filePath);

	//cout << bitBuffer << endl;
}

void test2DCT()
{
	size_t widthInBlocks = (256 + 7) / 8;
	size_t width = widthInBlocks * 8;

	size_t heightInBlocks = (256 + 7) / 8;
	size_t height = heightInBlocks * 8;

	size_t size = width * height;

	vector<float> testImage(size);
	vector<float> testDCTResult(size, 0);

	for (size_t i = 0; i < size; i++)
	{
		testImage[i] = static_cast<float>(i%256);
	}
	
	vector<PointerMatrix> testImageBlocks;
	vector<PointerMatrix> testImageResultBlocks;

	for (size_t x = 0; x < width; x += 8)
	{
		for (size_t curOffset = x; curOffset < size; curOffset += width * 8)
		{
			testImageBlocks.emplace_back(
				&testImage[curOffset],
				&testImage[curOffset + width * 1],
				&testImage[curOffset + width * 2],
				&testImage[curOffset + width * 3],
				&testImage[curOffset + width * 4],
				&testImage[curOffset + width * 5],
				&testImage[curOffset + width * 6],
				&testImage[curOffset + width * 7]
				);

			testImageResultBlocks.emplace_back(
				&testDCTResult[curOffset],
				&testDCTResult[curOffset + width * 1],
				&testDCTResult[curOffset + width * 2],
				&testDCTResult[curOffset + width * 3],
				&testDCTResult[curOffset + width * 4],
				&testDCTResult[curOffset + width * 5],
				&testDCTResult[curOffset + width * 6],
				&testDCTResult[curOffset + width * 7]
				);
		}
	}

	benchmark("Benchmark Direct DCT", 231, [&testImageBlocks, &testImageResultBlocks]()
	{
		for (size_t i = 0; i < testImageBlocks.size(); i++)
		{
			DCT::directDCT(testImageBlocks[i], testImageResultBlocks[i]);
		}
	});

	benchmark("Benchmark Seperate DCT", 5903, [&testImageBlocks, &testImageResultBlocks]()
	{
		for (size_t i = 0; i < testImageBlocks.size(); i++)
		{
			DCT::seperateDCT(testImageBlocks[i], testImageResultBlocks[i]);
		}
	});

	benchmark("Benchmark Arai DCT", 47393, [&testImageBlocks, &testImageResultBlocks]()
	{
		for (size_t i = 0; i < testImageBlocks.size(); i++)
		{
			DCT::araiDCT(testImageBlocks[i], testImageResultBlocks[i]);
		}
	});

	benchmark("Benchmark Arai DCT (AVX)", 147059, [&testImageBlocks, &testImageResultBlocks]()
	{
		for (size_t i = 0; i < testImageBlocks.size(); i++)
		{
			DCT::araiDCTAVX(testImageBlocks[i], testImageResultBlocks[i]);
		}
	});
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
		arr[i] = static_cast<float>(i+1);
	}

	float resultBytes[64];
	PointerMatrix result(resultBytes);
	mat8x8 matResult;

	//PointerMatrix testMatrix = PointerMatrix(arr);
	PointerMatrix testMatrix = PointerMatrix(rowOne, rowTwo, rowThree, rowFour, rowFive, rowSix, rowSeven, rowEight);
	auto kokMatrix = mat8x8(rowOne, rowTwo, rowThree, rowFour, rowFive, rowSix, rowSeven, rowEight);

	cout << "Input Block:" << endl;
	for (size_t i = 0; i < 8; i++)
	{
		for (size_t j = 0; j < 8; j++)
		{
			//cout << round(result[i][j]) << " | ";
			printf("%8.2f | ", roundf(testMatrix[i][j]));
		}
		cout << endl;
	}
	cout << "================" << endl;

	cout << "Start direct DCT" << endl;
	DCT::directDCT(testMatrix, result);
	cout << endl;
	for (size_t i = 0; i < 8; i++)
	{
		for (size_t j = 0; j < 8; j++)
		{
			printf("%8.2f | ", result[i][j]);
		}
		cout << endl;
	}
	cout << "End direct DCT" << endl;

	cout << "================" << endl;

	cout << "Start seperate DCT" << endl;
	DCT::seperateDCT(testMatrix, result);
	cout << endl;
	for (size_t i = 0; i < 8; i++)
	{
		for (size_t j = 0; j < 8; j++)
		{
			printf("%8.2f | ", result[i][j]);
		}
		cout << endl;
	}
	cout << "End seperate DCT" << endl;

	cout << "================" << endl;

	cout << "Start arai DCT" << endl;
	DCT::araiDCT(testMatrix, result);
	cout << endl;
	for (size_t i = 0; i < 8; i++)
	{
		for (size_t j = 0; j < 8; j++)
		{
			printf("%8.2f | ", result[i][j]);
		}
		cout << endl;
	}
	cout << "End arai DCT" << endl;

	cout << "================" << endl;

	cout << "Start arai DCT (AVX)" << endl;
	DCT::araiDCTAVX(testMatrix, result);
	cout << endl;
	for (size_t i = 0; i < 8; i++)
	{
		for (size_t j = 0; j < 8; j++)
		{
			printf("%8.2f | ", result[i][j]);
		}
		cout << endl;
	}
	cout << "End arai DCT (AVX)" << endl;

	cout << "================" << endl;

	cout << "Start direct IDCT" << endl;
	DCT::directIDCT(result, testMatrix);
	cout << endl;
	for (size_t i = 0; i < 8; i++)
	{
		for (size_t j = 0; j < 8; j++)
		{
			printf("%8.2f | ", testMatrix[i][j]);
		}
		cout << endl;
	}
	cout << "End direct IDCT" << endl;
}

void EncodeJPEG(string srcFile, string dstFile)
{
	StopWatch sw;

	SamplingDefinition scheme = Sampling::Scheme420;
	auto qtables = std::array<QTable, 3> { JPEGQuantization::luminance, JPEGQuantization::chrominance, JPEGQuantization::chrominance };

	ImagePtr image = nullptr;
	EncoderPtr encoder = nullptr;

	cout << "Load image file from: " << srcFile << endl;
		image = ImageLoader::Load(srcFile, scheme);
	sw("Load file");

	std::cout << "Create encoder object ";
		encoder = std::make_shared<Encoder>(image, qtables);
	sw();

	std::cout << "Convert image to YCbCr AVX ";
		encoder->convertToYCbCr();
	sw();

	std::cout << "Reduce channel resolution for scheme ";
		encoder->reduceResolutionBySchema();
	sw();

	std::cout << "Apply DCT ";
		encoder->applyDCT(YCbCrColorName::Y);
		encoder->applyDCT(YCbCrColorName::Cb);
		encoder->applyDCT(YCbCrColorName::Cr);
	sw();

	std::cout << "Save image file to: " << dstFile << std::endl;
		ImageLoader::Save(dstFile, encoder);
	sw("Save file");


	sw.stop();
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
	COMPILER_PREFACE;

	//for (int i = 0; i < 10000000; i++)
	//{
	//	testValues[i] = rand() % 2;
	//}

	//benchmark("bitBufferTest", 1, [&]() {
	//	bitBufferTest(argv[2]);
	//});

	//benchmark("testHuffmanEncoding", 1, [&]() {
	//	testHuffmanEncoding();
	//});
	//return  0;

	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <Source File> <Destination File>" << std::endl;
		return 1;
	}
	
	std::string srcFile(argv[1]);
	std::string dstFile(argv[2]);

	std::cout << "Test EncodeJPEG" << endl;
	EncodeJPEG(srcFile, dstFile);
		
	//std::cout << "Test DCT" << endl;
	//benchmark("Test DCT", 1, [&]()
	//{
	//	test2DCT();
	//});

	return 0;
}