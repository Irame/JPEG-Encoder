#pragma once
#include "Image.h"
#include "SamplingScheme.h"

class ImageLoader
{
public:
	ImageLoader();
	~ImageLoader();

	static ImageCCPtr Load(const std::string& filename, SamplingScheme scheme, QTable luminance, QTable chrominance);
	static void     Save(const std::string& filename, ImageCCPtr image);

	static ImageCCPtr LoadPPM(std::string path, SamplingScheme scheme, QTable luminance, QTable chrominance);
	static ImageCCPtr LoadPNG(std::string path, SamplingScheme scheme, QTable luminance, QTable chrominance);

	static void SavePPM(std::string path, ImageCCPtr image);
	static void SavePNG(std::string path, ImageCCPtr image);
	static void SaveJPG(std::string path, ImageCCPtr image);

private:
	static inline int clamp(int lower, int x, int upper);
	static inline std::string fileExtension(const std::string& filename);
};

