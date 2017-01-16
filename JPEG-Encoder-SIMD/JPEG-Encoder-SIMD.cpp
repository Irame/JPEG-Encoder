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
#include "boost/program_options/options_description.hpp"
#include "boost/program_options/parsers.hpp"
#include "boost/program_options/variables_map.hpp"

#define CL_HPP_TARGET_OPENCL_VERSION 120

#include "OpenCL.h"

using namespace std;
namespace po = boost::program_options;

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

bool stringContains(string s, string value)
{
	return s.find(value) != string::npos;
}

bool stringContainsOrEmpty(string s, string value)
{
    return s.empty() || stringContains(s, value);
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
	JPEGSegments::StartOfFrame0 startOfFrame0(1680,900, Sampling::Scheme422);

	vector<byte> allSymbols{ 0, 1, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6 };
	std::cout << "All symbols: " << allSymbols << endl;

	auto huffmanTable = HuffmanTable<byte>::create(16, allSymbols);
	std::cout << "Huffman Table:" << endl << *huffmanTable;

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

void benchmarkDCT(string benchmarkNames, int openclPlatform, int openclDevice)
{
	size_t widthInBlocks = (256 + 7) / 8;
	size_t width = widthInBlocks * 8;

	size_t heightInBlocks = (256 + 7) / 8;
	size_t height = heightInBlocks * 8;

	size_t size = width * height;

	vector<float> testImage(size);

    if (stringContainsOrEmpty(benchmarkNames, "direct")) {
        for (size_t i = 0; i < size; i++)
        {
            testImage[i] = static_cast<float>(i % 256);
        }

        benchmark("Benchmark Direct DCT", 444, [&testImage, &width, &size]()
        {
#pragma omp parallel for
            for (int x = 0; x < width; x += 8)
            {
                for (size_t curOffset = x; curOffset < size; curOffset += width * 8)
                {
                    PointerMatrix curBlock(
                        &testImage[curOffset],
                        &testImage[curOffset + width * 1],
                        &testImage[curOffset + width * 2],
                        &testImage[curOffset + width * 3],
                        &testImage[curOffset + width * 4],
                        &testImage[curOffset + width * 5],
                        &testImage[curOffset + width * 6],
                        &testImage[curOffset + width * 7]
                    );

                    DCT::directDCT(curBlock, curBlock);
                }
            }
        });
    }

    if (stringContainsOrEmpty(benchmarkNames, "seperate")) {
        for (size_t i = 0; i < size; i++)
        {
            testImage[i] = static_cast<float>(i % 256);
        }

        benchmark("Benchmark Seperate DCT", 2680, [&testImage, &width, &size]()
        {
#pragma omp parallel for
            for (int x = 0; x < width; x += 8)
            {
                for (size_t curOffset = x; curOffset < size; curOffset += width * 8)
                {
                    PointerMatrix curBlock(
                        &testImage[curOffset],
                        &testImage[curOffset + width * 1],
                        &testImage[curOffset + width * 2],
                        &testImage[curOffset + width * 3],
                        &testImage[curOffset + width * 4],
                        &testImage[curOffset + width * 5],
                        &testImage[curOffset + width * 6],
                        &testImage[curOffset + width * 7]
                    );

                    DCT::seperateDCT(curBlock, curBlock);
                }
            }
        });
    }

    if (stringContainsOrEmpty(benchmarkNames, "arai")) {
        for (size_t i = 0; i < size; i++)
        {
            testImage[i] = static_cast<float>(i % 256);
        }

        benchmark("Benchmark Arai DCT", 109890, [&testImage, &width, &size]()
        {
#pragma omp parallel for
            for (int x = 0; x < width; x += 8)
            {
                for (size_t curOffset = x; curOffset < size; curOffset += width * 8)
                {
                    PointerMatrix curBlock(
                        &testImage[curOffset],
                        &testImage[curOffset + width * 1],
                        &testImage[curOffset + width * 2],
                        &testImage[curOffset + width * 3],
                        &testImage[curOffset + width * 4],
                        &testImage[curOffset + width * 5],
                        &testImage[curOffset + width * 6],
                        &testImage[curOffset + width * 7]
                    );

                    DCT::araiDCT(curBlock, curBlock);
                }
            }
        });
    }

    if (stringContainsOrEmpty(benchmarkNames, "opencl")) {
        for (size_t i = 0; i < size; i++)
        {
            testImage[i] = static_cast<float>(i % 256);
        }

        OpenCL clDct(width, height, openclPlatform, openclDevice);
        benchmark("Benchmark Arai DCT (OpenCL)", 129870, [&testImage, &width, &size, &clDct]()
        {
			clDct.enqueueWriteImage(testImage.data());
			clDct.enqueueExecuteDCT();
			clDct.enqueueReadImage(testImage.data());
            clDct.finishQueue();
		});
        clDct.finishQueue();
    }

    if (stringContainsOrEmpty(benchmarkNames, "avx")) {
        for (size_t i = 0; i < size; i++)
        {
            testImage[i] = static_cast<float>(i % 256);
        }

        benchmark("Benchmark Arai DCT (AVX)", 370370, [&testImage, &width, &size]()
        {
#ifdef AVX512
#pragma omp parallel for
            for (int x = 0; x < width; x += 16)
            {
                for (size_t curOffset = x; curOffset < size; curOffset += width * 8)
                {
                    PointerMatrix curBlock1(
                        &testImage[curOffset],
                        &testImage[curOffset + width * 1],
                        &testImage[curOffset + width * 2],
                        &testImage[curOffset + width * 3],
                        &testImage[curOffset + width * 4],
                        &testImage[curOffset + width * 5],
                        &testImage[curOffset + width * 6],
                        &testImage[curOffset + width * 7]
                    );

                    PointerMatrix curBlock2(
                        &testImage[curOffset],
                        &testImage[curOffset + 8 + width * 1],
                        &testImage[curOffset + 8 + width * 2],
                        &testImage[curOffset + 8 + width * 3],
                        &testImage[curOffset + 8 + width * 4],
                        &testImage[curOffset + 8 + width * 5],
                        &testImage[curOffset + 8 + width * 6],
                        &testImage[curOffset + 8 + width * 7]
                    );

                    DCT::araiDCTAVX(curBlock1, curBlock2, curBlock1, curBlock2);
                }
            }
#else
#pragma omp parallel for
            for (int x = 0; x < width; x += 8)
            {
                for (size_t curOffset = x; curOffset < size; curOffset += width * 8)
                {
                    PointerMatrix curBlock(
                        &testImage[curOffset],
                        &testImage[curOffset + width * 1],
                        &testImage[curOffset + width * 2],
                        &testImage[curOffset + width * 3],
                        &testImage[curOffset + width * 4],
                        &testImage[curOffset + width * 5],
                        &testImage[curOffset + width * 6],
                        &testImage[curOffset + width * 7]
                    );

                    DCT::araiDCTAVX(curBlock, curBlock);
                }
            }
#endif 
        });
    }
}

void printMatrix8x8(float* matrix)
{
    for (size_t i = 0; i < 8; i++)
    {
        for (size_t j = 0; j < 8; j++)
        {
            printf("%8.2f", matrix[i * 8 + j]);
            if (j < 7) cout << " | ";
        }
        std::cout << endl;
    }
}

bool matricesEqual(float* mat1, float* mat2)
{
    for (int i = 0; i < 64; i++)
    {
        if (abs(mat1[i] - mat2[i]) < 0.001)
            return true;
    }
    return false;
}

void ceckTest(float* result, float* reference)
{
    if (!matricesEqual(result, reference)) {
        cout << "Test failed!" << endl;
        printMatrix8x8(result);
    }
    else {
        cout << "Test successful!" << endl;
    }
}

void testDCT(string testNames, int openclPlatform, int openclDevice)
{
	cout << "Start dct Test for: " << testNames << endl;
	float testMatrixBytes[] = { 
	 140, 144, 147, 140, 140, 155, 179, 175,
	 144, 152, 140, 147, 140, 148, 167, 179,
	 152, 155, 136, 167, 163, 162, 152, 172,
	 168, 145, 156, 160, 152, 155, 136, 160,
	 162, 148, 156, 148, 140, 136, 147, 162,
	 147, 167, 140, 155, 155, 140, 136, 162,
	 136, 156, 123, 167, 162, 144, 140, 147,
	 148, 155, 136, 155, 152, 147, 147, 136 };

    int32_t dctResultBytes[] = {
        0x4339fff8, 0xc18ff9e3, 0x416c7824, 0xc10fac32, 0x41ba0015, 0xc113b983, 0xc15f81cf, 0xc1977f72,
        0x41a44e43, 0xc2085f2d, 0x41d2a4b0, 0xc1109dd4, 0xc12eed8c, 0x412bb0dc, 0x415c5bfb, 0x40de8d74,
        0xc1262488, 0xc1bc1cd3, 0xbfed40e0, 0x40c14b78, 0xc190989b, 0x404c9b20, 0xc1a3568f, 0xbf539428,
        0xc101ad4f, 0xc0a15388, 0x41655078, 0xc169d07c, 0xc1037c40, 0xc02edf7c, 0xc0456874, 0x4106ddca,
        0xc04fff8c, 0x411805e6, 0x40fc4f06, 0x3fa89490, 0xc12ffff8, 0x418f3c64, 0x41930e2c, 0x4173dbc4,
        0x4076bf65, 0xc00dbd40, 0xc19155ee, 0x4107ff61, 0x41044d11, 0xc066f07c, 0x3f5e7628, 0xc0db99ca,
        0x410e6ba6, 0x3f220c40, 0xc03ab4a0, 0x40690c2c, 0xbf961290, 0xc0ed7f67, 0xbf92bf58, 0xbff65830,
        0x3d489070, 0xc0fa0408, 0xc01b2b3c, 0x3fcb9142, 0x3f99812c, 0x4087e78a, 0xc0cd5b69, 0x3ea12bac
    };

    float *dctResultBytesFloat = reinterpret_cast<float*>(dctResultBytes);

	float resultBytes[64];
	PointerMatrix result(resultBytes);
    float resultBytes2[64];
    PointerMatrix result2(resultBytes2);
	mat8x8 matResult;

	PointerMatrix testMatrix = PointerMatrix(testMatrixBytes);

    if (stringContainsOrEmpty(testNames, "direct")) {
        std::cout << "Start direct DCT" << endl;
        DCT::directDCT(testMatrix, result);
        ceckTest(resultBytes, dctResultBytesFloat);
        std::cout << "End direct DCT" << endl;
        std::cout << "================" << endl;
    }

    if (stringContainsOrEmpty(testNames, "seperate")) {
        std::cout << "Start seperate DCT" << endl;
        DCT::seperateDCT(testMatrix, result);
        ceckTest(resultBytes, dctResultBytesFloat);
        std::cout << "End seperate DCT" << endl;
        std::cout << "================" << endl;
    }

    if (stringContainsOrEmpty(testNames, "arai")) {
        std::cout << "Start arai DCT" << endl;
        DCT::araiDCT(testMatrix, result);
        ceckTest(resultBytes, dctResultBytesFloat);
        std::cout << "End arai DCT" << endl;
        std::cout << "================" << endl;
    }

    if (stringContainsOrEmpty(testNames, "opencl")) {
        std::cout << "Start arai2 DCT" << endl;
        OpenCL clDct(8, 8, openclPlatform, openclDevice);
        clDct.enqueueWriteImage(testMatrixBytes);
        clDct.enqueueExecuteDCT();
        clDct.enqueueReadImage(resultBytes);
        clDct.finishQueue();
        ceckTest(resultBytes, dctResultBytesFloat);
        std::cout << "End arai2 DCT" << endl;
        std::cout << "================" << endl;
    }

    if (stringContainsOrEmpty(testNames, "avx")) {
        std::cout << "Start arai DCT (AVX)" << endl;
#ifdef AVX512
        DCT::araiDCTAVX(testMatrix, testMatrix, result, result2);
        std::cout << endl;
        if (!matricesEqual(resultBytes, dctResultBytesFloat) || !matricesEqual(resultBytes2, dctResultBytesFloat)) {
            cout << "Test failed!" << endl;
            printMatrix8x8(resultBytes);
            printMatrix8x8(resultBytes2);
        }
        else {
            cout << "Test successful!" << endl;
        }
#else
        DCT::araiDCTAVX(testMatrix, result);
        ceckTest(resultBytes, dctResultBytesFloat);
#endif
        std::cout << "End arai DCT (AVX)" << endl;
        std::cout << "================" << endl;
    }

    if (stringContainsOrEmpty(testNames, "directidct")) {
        std::cout << "Start direct IDCT" << endl;
        DCT::directIDCT(dctResultBytesFloat, result);
        ceckTest(resultBytes, testMatrixBytes);
        std::cout << "End direct IDCT" << endl;
    }
}

void EncodeJPEG(string srcFile, string dstFile)
{
	StopWatch sw;

	SamplingDefinition scheme = Sampling::Scheme420;
	QTableSet qtables(JPEGQuantization::luminance, JPEGQuantization::chrominance);

	ImagePtr image = nullptr;
	EncoderPtr encoder = nullptr;

	std::cout << "Load image file from: " << srcFile << endl;
		image = ImageLoader::Load(srcFile, scheme);
	sw("Load file");

	if (!image)
	{
		std::cout << "Failed to load source image " << srcFile << endl;
		abort();
	}

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
	std::cout << "All symbols: " << allSymbols << endl;
	
	auto huffmanTable = HuffmanTable<byte>::create(16, allSymbols);
	std::cout << "Huffman Table:" << endl << *huffmanTable;

	vector<byte> encodeDecodeTestSymbols{ 3, 0, 4 };
	std::cout << "Test symbols: " << encodeDecodeTestSymbols << endl;

	auto encodedSymbols = huffmanTable->encode(encodeDecodeTestSymbols);
	std::cout << "Encoded symbols: " << *encodedSymbols << endl;

	vector<byte> decodedSymbols = huffmanTable->decode(encodedSymbols);

	std::cout << "Decoded symbols: " << decodedSymbols << endl;
}

int main(int argc, char* argv[])
{
	COMPILER_PREFACE;

	string mode;
	string srcFile;
	string dstFile;
    string modules;
    string dctImps;
	int openclPlatform;
	int openclDevice;
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "Display help message")
		("opencl-info", "OpenCL Platform info")
		("mode", po::value<string>(&mode), "Choose the mode: convert, test, benchmark")
		("src-file", po::value<string>(&srcFile), "File to read (Mode: convert)")
		("dst-file", po::value<string>(&dstFile), "File to write (Mode: convert)")
        ("modules", po::value<string>(&modules)->default_value("dct"), "Comma seperated List of modules to test/benchmark (Mode: test, benchmark)")
        ("dct-imps", po::value<string>(&dctImps), "Comma seperated List of dct-implementations to test/benchmark (Mode: test, benchmark)")
		("opencl-platform", po::value<int>(&openclPlatform)->default_value(0), "OpenCL Platform number (zero based) (Mode: test[dct], benchmark[dct])")
		("opencl-device", po::value<int>(&openclDevice)->default_value(0), "OpenCL Device number (zero based) (Mode: test[dct], benchmark[dct])")
	;

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
	po::notify(vm);

	if (vm.count("opencl-info")) {
		OpenCL::ReadOpenCLPlatforms();
	}


	if (vm.count("help")) {
		cout << desc << "\n";
		return 1;
	}

	if (mode == "convert")
	{
		if (!vm.count("src-file") || !vm.count("dst-file"))
			cout << "The convert mode needs src-file and dst-file." << endl; 
		else
			EncodeJPEG(srcFile, dstFile);
	} else if (mode == "test") {
        if (stringContains(modules, "dct"))
            testDCT(dctImps, openclPlatform, openclDevice);
        if (stringContains(modules, "huffman"))
            testHuffmanEncoding();
        if (stringContains(modules, "bitBuffer"))
            bitBufferTest(dstFile);
	}
	else if (mode == "benchmark") {
		if (stringContains(modules, "dct"))
			benchmarkDCT(dctImps, openclPlatform, openclDevice);
	} else {
        cout << desc << "\n";
        return 1;
	}

	return 0;
}