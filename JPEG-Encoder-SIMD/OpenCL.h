#pragma once

#include "CL/cl.hpp"
#include "QuantizationTables.h"

class OpenCL
{
private:
	cl::Context context;
	cl::CommandQueue queue;
	cl::Program program;

	cl::Buffer imageBuffer;
    cl::LocalSpaceArg localBuffer;
    cl::Buffer qTableBuffer;

	int width, height;

    cl::Kernel transpose8;
    cl::Kernel oneDimDct;
    cl::Kernel dctNormalize;

	cl::Kernel twoDimDct;

    cl::Kernel quantize;
public:

    OpenCL(int width, int height, int platform = 0, int device = 0);

	void enqueueWriteImage(float* image) const;
	void enqueueReadImage(float* image) const;
    void enqueueWriteQTable(const QTable& qTable) const;

	void enqueueExecuteDCT() const;
    void executeQuantization() const;

    void finishQueue() const;
};
