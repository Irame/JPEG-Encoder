#pragma once
#include "Image.h"
#include "SamplingScheme.h"
#include "Encoder.h"

class ImageLoader
{
public:
	ImageLoader();
	~ImageLoader();

	static ImagePtr Load(const std::string& filename, SamplingScheme scheme);
	static void     Save(const std::string& filename, ImagePtr image);

	static ImagePtr LoadPPM(std::string path, SamplingScheme scheme);
	static ImagePtr LoadPNG(std::string path, SamplingScheme scheme);

	static void SavePPM(std::string path, ImagePtr image);
	static void SavePNG(std::string path, ImagePtr image);
	static void SaveJPG(std::string path, EncoderPtr image);

private:
	static inline int clamp(int lower, int x, int upper);
	static inline std::string fileExtension(const std::string& filename);
};

