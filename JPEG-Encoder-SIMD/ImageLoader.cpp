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

ImageCCPtr ImageLoader::Load(const std::string& filename, SamplingScheme scheme, QTable luminance, QTable chrominance)
{
	std::string ext = fileExtension(filename);

	if (ext == "png") {
		return LoadPNG(filename, scheme, luminance, chrominance);
	} else if (ext == "ppm") {
		return LoadPPM(filename, scheme, luminance, chrominance);
	} else {
		std::cout << "Failed to load image. Unknown file extension " << ext << std::endl;
		return nullptr;
	}
}

void ImageLoader::Save(const std::string& filename, ImageCCPtr image)
{
	std::string ext = fileExtension(filename);

	if (ext == "png") {
		return SavePNG(filename, image);
	} else if (ext == "ppm") {
		return SavePPM(filename, image);
	} else if (ext == "jpg" || ext == "jpeg"){
		return SaveJPG(filename, image);
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


ImageCCPtr ImageLoader::LoadPPM(std::string path, SamplingScheme scheme, QTable luminance, QTable chrominance)
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

	ImageCCPtr resultImage = make_shared<Image>(width, height, scheme, luminance, chrominance);
	resultImage->setRawPixelData((float*)&data[0]);
	return resultImage;
}

void ImageLoader::SavePPM(std::string path, ImageCCPtr image)
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

ImageCCPtr ImageLoader::LoadPNG(std::string path, SamplingScheme samplingScheme, QTable luminance, QTable chrominance)
{
	std::vector<unsigned char> imgData;
	unsigned imgWidth, imgHeight;

	unsigned error = lodepng::decode(imgData, imgWidth, imgHeight, path);
	if (error) {
		std::cout << "Failed to decode png " << path << " with error: " << error << ": " << lodepng_error_text(error) << std::endl;
		return nullptr;
	}

	std::vector<float> imgDataFloat(imgData.size());
	for (size_t i = 0; i < imgDataFloat.size(); i++) {
		imgDataFloat[i] = imgData[i] / 255.0f;
	}

	ImageCCPtr resultImage = make_shared<Image>(imgWidth, imgHeight, samplingScheme, luminance, chrominance);
	resultImage->setRawPixelData((float*)&imgDataFloat[0]);
	return resultImage;
}

void ImageLoader::SavePNG(std::string path, ImageCCPtr image)
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
void ImageLoader::SaveJPG(std::string path, ImageCCPtr image) {
	const Dimension2D& imageSize = image->getImageSize();
	const SamplingScheme& scheme = image->getSamplingScheme();
	const HuffmanTablePtr<byte> huffmann = image->getHuffmanTable();

	BitBuffer bitBuffer;

	JPEGSegments::StartOfImage startOfImage;
	JPEGSegments::APP0 app0;
	JPEGSegments::StartOfFrame0 startOfFrame0(static_cast<short>(imageSize.width), static_cast<short>(imageSize.height), scheme);
	JPEGSegments::EndOfImage endOfImage;
	JPEGSegments::DefineHuffmannTable defineHuffmannTable(byte(0), JPEGSegments::HuffmanTableType::DC, *huffmann);
	JPEGSegments::DefineQuantizationTable defineQuantizationTableLuminance(JPEGSegments::QuantizationTableType::Luminance, image->getLuminanceQTable());
	JPEGSegments::DefineQuantizationTable defineQuantizationTableChrominance(JPEGSegments::QuantizationTableType::Chrominance, image->getChrominanceQTable());

	JPEGSegments::Serialize(startOfImage, bitBuffer);
	JPEGSegments::Serialize(app0, bitBuffer);
	JPEGSegments::Serialize(startOfFrame0, bitBuffer);
	JPEGSegments::Serialize(defineHuffmannTable, bitBuffer);
	JPEGSegments::Serialize(defineQuantizationTableLuminance, bitBuffer);
	JPEGSegments::Serialize(defineQuantizationTableChrominance, bitBuffer);
	JPEGSegments::Serialize(endOfImage, bitBuffer);

	bitBuffer.writeToFile(path);
}
