#pragma once

#include <string>

#include "Image.h"

class ImageLoader
{
public:
	ImageLoader();
	~ImageLoader();

	static ImagePtr Load(const std::string& filename, size_t stepX, size_t stepY);
	static void     Save(const std::string& filename, ImagePtr image);

	static ImagePtr LoadPPM(std::string path, size_t stepX, size_t stepY);
	static ImagePtr LoadPNG(std::string path, size_t stepX, size_t stepY);

	static void SavePPM(std::string path, ImagePtr image);
	static void SavePNG(std::string path, ImagePtr image);

private:
	static inline int clamp(int lower, int x, int upper);
	static inline std::string fileExtension(const std::string& filename);
};

