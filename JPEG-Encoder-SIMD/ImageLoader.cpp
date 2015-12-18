#include "stdafx.h"
#include "ImageLoader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include "SIMD.h"
#include "lodepng.h"
#include "Benchmark.h"
#include "JPEGSegments.h"
#include "BitBuffer.h"

using namespace std;

ImageLoader::ImageLoader()
{
}

ImageLoader::~ImageLoader()
{
}

ImagePtr ImageLoader::Load(const std::string& filename, SamplingDefinition scheme)
{
	std::string ext = fileExtension(filename);

	if (ext == "png") {
		return LoadPNG(filename, scheme);
	} else if (ext == "ppm") {
		return LoadPPM(filename, scheme);
	} else {
		std::cout << "Failed to load image. Unknown file extension " << ext << std::endl;
		return nullptr;
	}
}

void ImageLoader::Save(const std::string& filename, ImagePtr image)
{
	std::string ext = fileExtension(filename);

	if (ext == "png") {
		return SavePNG(filename, image);
	} else if (ext == "ppm") {
		return SavePPM(filename, image);
	} else if (ext == "jpg" || ext == "jpeg"){
		return SaveJPG(filename, std::static_pointer_cast<Encoder, Image>(image));
	} else {
		std::cout << "Failed to save image. Unknown file extension " << ext << std::endl;
	}
}

inline int ImageLoader::clamp(int lower, int x, int upper)
{
	return max(lower, min(x, upper));
}

inline std::string ImageLoader::fileExtension(const std::string& filename)
{
	return filename.substr(filename.find_last_of(".") + 1);
}


ImagePtr ImageLoader::LoadPPM(std::string path, SamplingDefinition scheme)
{
	enum State {
		None, Size, Pixels
	};

	State state = State::None;
	ifstream fileStream = ifstream(path);
	string line;
	float maxValue = 1.0f;
	int width = 0;
	int height = 0;
	int currentWidth = 0;
	int currentHeight = 0;
	int j = 0;
	vector<PixelData32> data;

	while (getline(fileStream, line)) {
		if (line == "P3")
			continue;
		if (line.empty())
			continue;
		if (line.front() == '#')
			continue;
		if (state == State::None) {
			std::stringstream stringStream(line);
			string size;
			for (int i = 0; getline(stringStream, size, ' '); i++) {
				if (i == 0)
					width = stoi(size);
				else {
					height = stoi(size);
					//break;
				}
			}
			data.resize(width*height);

			state = Size;
		}
		else if (state == State::Size) {
			maxValue = static_cast<float>(stoi(line));
			state = State::Pixels;
		}
		else if (state == State::Pixels) {
			std::stringstream stringStream(line);
			string color;
			while (getline(stringStream, color, ' ')) {
				if (color.length() == 0)
					continue;

				switch (j)
				{
				case 0: data[currentHeight * width + currentWidth].R = stoi(color) / maxValue; break;
				case 1: data[currentHeight * width + currentWidth].G = stoi(color) / maxValue; break;
				case 2: data[currentHeight * width + currentWidth].B = stoi(color) / maxValue; break;
				default: break;
				}

				j++;
				if (j > 2) {
					j = 0;
					currentWidth++;
					if (currentWidth >= width) {
						currentWidth = 0;
						currentHeight++;
					}
				}
			}
		}
	}


	ImagePtr resultImage = make_shared<Image>(width, height, scheme);
	resultImage->setRawPixelData(reinterpret_cast<float*>(data.data()));
	return resultImage;
}

void ImageLoader::SavePPM(std::string path, ImagePtr image)
{
	ofstream fileStream = ofstream(path);

	const Dimension2D& imageSize = image->getImageSize();
	const int maxVal = 255;

	fileStream << "P3" << endl;
	fileStream << imageSize.width << " " << imageSize.height << endl;
	fileStream << maxVal << endl;

	PixelData32 pixel;
	
	for (size_t y = 0; y < imageSize.height; y++)
	{
		for (size_t x = 0; x < imageSize.width; x++)
		{
			image->getPixel(pixel, x, y);
			const int r = clamp(0, static_cast<int>(pixel.R * maxVal + 0.5f), maxVal);
			const int g = clamp(0, static_cast<int>(pixel.G * maxVal + 0.5f), maxVal);
			const int b = clamp(0, static_cast<int>(pixel.B * maxVal + 0.5f), maxVal);

			fileStream << r << " " << g << " " << b << "  ";
		}
		fileStream << endl;
	}
}

ImagePtr ImageLoader::LoadPNG(std::string path, SamplingDefinition samplingScheme)
{
	std::vector<unsigned char> imgData;
	unsigned imgWidth, imgHeight;

	unsigned error = lodepng::decode(imgData, imgWidth, imgHeight, path, LCT_RGB);
	if (error) {
		std::cout << "Failed to decode png " << path << " with error: " << error << ": " << lodepng_error_text(error) << std::endl;
		return nullptr;
	}

	std::vector<float> imgDataFloat(imgData.size());
	for (size_t i = 0; i < imgDataFloat.size(); i++) {
		imgDataFloat[i] = imgData[i] / 255.0f;
	}

	ImagePtr resultImage = make_shared<Image>(imgWidth, imgHeight, samplingScheme);
	resultImage->setRawPixelData(imgDataFloat.data());
	return resultImage;
}

void ImageLoader::SavePNG(std::string path, ImagePtr image)
{
	const Dimension2D& simulatedSize = image->getSimulatedSize();

	std::vector<float> imageData = image->getRawPixelDataSimulated();

	std::vector<unsigned char> imgData(imageData.size());
	for (size_t i = 0; i < imgData.size(); i++) {
		imgData[i] = clamp(0, static_cast<int>(imageData[i] * 255 + 0.5f), 255);
	}

	unsigned error = lodepng::encode(path, imgData, (unsigned int)simulatedSize.width, (unsigned int)simulatedSize.height);
	if (error) {
		std::cout << "Failed to encode png " << path << " with error: " << error << ": " << lodepng_error_text(error) << std::endl;
	}
}
void ImageLoader::SaveJPG(std::string path, EncoderPtr image) {
	const Dimension2D& imageSize = image->getImageSize();
	const SamplingDefinition& scheme = image->getSamplingScheme();
	const HuffmanTablePtr<byte> huffmannDCY = image->getHuffmanTable(Encoder::CoefficientType::DC, YCbCrColorName::Y);
	const HuffmanTablePtr<byte> huffmannDCCb = image->getHuffmanTable(Encoder::CoefficientType::DC, YCbCrColorName::Cb);
	const HuffmanTablePtr<byte> huffmannACY = image->getHuffmanTable(Encoder::CoefficientType::AC, YCbCrColorName::Y);
	const HuffmanTablePtr<byte> huffmannACCb = image->getHuffmanTable(Encoder::CoefficientType::AC, YCbCrColorName::Cb);
	auto acHT = std::array<byte, 3>();
	auto dcHT = std::array<byte, 3>();
	BitBuffer bitBuffer;

	JPEGSegments::StartOfImage startOfImage;
	JPEGSegments::APP0 app0;
	JPEGSegments::StartOfFrame0 startOfFrame0(static_cast<short>(imageSize.width), static_cast<short>(imageSize.height), scheme);
	JPEGSegments::DefineHuffmannTable defineHuffmannTableDCY(YCbCrColorName::Y, JPEGSegments::HuffmanTableType::DC, *huffmannDCY);
	JPEGSegments::DefineHuffmannTable defineHuffmannTableDCCb(YCbCrColorName::Cb, JPEGSegments::HuffmanTableType::DC, *huffmannDCCb);
	JPEGSegments::DefineHuffmannTable defineHuffmannTableACY(YCbCrColorName::Y, JPEGSegments::HuffmanTableType::AC, *huffmannACY);
	JPEGSegments::DefineHuffmannTable defineHuffmannTableACCb(YCbCrColorName::Cb, JPEGSegments::HuffmanTableType::AC, *huffmannACCb);
	JPEGSegments::DefineQuantizationTable defineQuantizationTableLuminance(YCbCrColorName::Y, image->getQTable(YCbCrColorName::Y));
	JPEGSegments::DefineQuantizationTable defineQuantizationTableChrominance(YCbCrColorName::Cb, image->getQTable(YCbCrColorName::Cb));
	JPEGSegments::DefineQuantizationTable defineQuantizationTableChrominance2(YCbCrColorName::Cr, image->getQTable(YCbCrColorName::Cr));
	
	acHT[YCbCrColorName::Y] = YCbCrColorName::Y;
	acHT[YCbCrColorName::Cb] = YCbCrColorName::Cb;
	acHT[YCbCrColorName::Cr] = YCbCrColorName::Cb;
	dcHT[YCbCrColorName::Y] = YCbCrColorName::Y;
	dcHT[YCbCrColorName::Cb] = YCbCrColorName::Cb;
	dcHT[YCbCrColorName::Cr] = YCbCrColorName::Cb;
	
	JPEGSegments::StartOfScan startOfScan(acHT, dcHT);
	JPEGSegments::EndOfImage endOfImage;


	JPEGSegments::Serialize(startOfImage, bitBuffer);
	JPEGSegments::Serialize(app0, bitBuffer);
	JPEGSegments::Serialize(startOfFrame0, bitBuffer);
	JPEGSegments::Serialize(defineHuffmannTableDCY, bitBuffer);
	JPEGSegments::Serialize(defineHuffmannTableDCCb, bitBuffer);
	JPEGSegments::Serialize(defineHuffmannTableACY, bitBuffer);
	JPEGSegments::Serialize(defineHuffmannTableACCb, bitBuffer);
	JPEGSegments::Serialize(defineQuantizationTableLuminance, bitBuffer);
	JPEGSegments::Serialize(defineQuantizationTableChrominance, bitBuffer);
	JPEGSegments::Serialize(defineQuantizationTableChrominance2, bitBuffer);
	JPEGSegments::Serialize(startOfScan, bitBuffer);

	image->serialize(bitBuffer);

	JPEGSegments::Serialize(endOfImage, bitBuffer);

	bitBuffer.writeToFile(path);
}
