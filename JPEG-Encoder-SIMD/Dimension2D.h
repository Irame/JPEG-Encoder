#pragma once

struct Dimension2D
{
	size_t width;
	size_t height;

	Dimension2D() : width(0), height(0) {}

	Dimension2D(size_t width, size_t height)
		: width(width), height(height)
	{}

	bool operator==(const Dimension2D& other) const
	{
		return other.width == width && other.height == height;
	}
};