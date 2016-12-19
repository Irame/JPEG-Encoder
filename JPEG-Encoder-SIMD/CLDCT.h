#pragma once

#include "CL/cl.hpp"

class CLDCT
{
private:
	cl::Context context;
	cl::CommandQueue queue;
	cl::Program program;
	cl::Buffer bufferImage;

	int width, height;
public:

	CLDCT(int width, int height);

	void writeBuffer(float* image);
	void readBuffer(float* image);
	void execute();
};
