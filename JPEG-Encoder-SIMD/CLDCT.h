#pragma once

#include "CL/cl.hpp"

class CLDCT
{
private:
	cl::Context context;
	cl::CommandQueue queue;
	cl::Program program;

	cl::Buffer bufferImage;
    cl::LocalSpaceArg localBuffer;

	int width, height;

    cl::Kernel transpose8;
    cl::Kernel oneDimDct;
    cl::Kernel dctNormalize;
public:

	CLDCT(int width, int height);

	void writeBuffer(float* image);
	void readBuffer(float* image);
	void execute();
};
