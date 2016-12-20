#include "CLDCT.h"

#include <iostream>
#include "CL/cl.hpp"
#include <fstream>
#include <time.h>

#include "Resource.h"

std::string read_source(const char *filename)
{
	std::string src;
	std::ifstream file(filename);
	file.seekg(0, std::ios::end);
	src.resize(file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(&src[0], src.length());
	return src;
};


CLDCT::CLDCT(int width, int height)
{
	this->width = width;
	this->height = height;

	// Init OpenCL program

	//get all platforms (drivers)
	std::vector<cl::Platform> all_platforms;
	cl::Platform::get(&all_platforms);
	if (all_platforms.size() == 0) {
		std::cout << " No platforms found. Check OpenCL installation!\n";
		exit(1);
	}
	cl::Platform default_platform = all_platforms[0];
	std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";

	//get default device of the default platform
	std::vector<cl::Device> all_devices;
	default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
	if (all_devices.size() == 0) {
		std::cout << " No devices found. Check OpenCL installation!\n";
		exit(1);
	}
	cl::Device default_device = all_devices[0];
	std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";


	context = cl::Context({ default_device });

	cl::Program::Sources sources;

	//Resource TwoDimDctCLFile = LOAD_RESOURCE(TwoDimDct_cl);
	//sources.push_back({ TwoDimDctCLFile.data(), TwoDimDctCLFile.size() });

	Resource Transpose8CLFile = LOAD_RESOURCE(Transpose8_cl);
	sources.push_back({ Transpose8CLFile.data(), Transpose8CLFile.size() });

	Resource OneDimDctCLFile = LOAD_RESOURCE(OneDimDct_cl);
	sources.push_back({ OneDimDctCLFile.data(), OneDimDctCLFile.size() });

	Resource DctNormalizeCLFile = LOAD_RESOURCE(DctNormalize_cl);
	sources.push_back({ DctNormalizeCLFile.data(), DctNormalizeCLFile.size() });

	program = cl::Program(context, sources);
	if (program.build({ default_device }) != CL_SUCCESS) {
		std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\\n";
		exit(1);
	}

	// create buffers on the device
	bufferImage = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * width * height);
    localBuffer = cl::LocalSpaceArg{sizeof(float) * 8 * 8};

	//create queue to which we will push commands for the device.
	queue = cl::CommandQueue(context, default_device);

    transpose8 = cl::Kernel(program, "transpose8");
    transpose8.setArg(0, bufferImage);
    transpose8.setArg(1, width);
    transpose8.setArg(2, localBuffer);

    oneDimDct = cl::Kernel(program, "oneDimDct");
    oneDimDct.setArg(0, bufferImage);
    oneDimDct.setArg(1, width);

    dctNormalize = cl::Kernel(program, "dctNormalize");
    dctNormalize.setArg(0, bufferImage);
    dctNormalize.setArg(1, width);
}

void CLDCT::writeBuffer(float* image)
{
	//write arrays A and B to the device
	cl_int error = queue.enqueueWriteBuffer(bufferImage, CL_TRUE, 0, sizeof(float) * width * height, image);

}

void CLDCT::readBuffer(float* image)
{
	memset(image, 42, sizeof(float)*width*height);
	cl_int error = queue.enqueueReadBuffer(bufferImage, CL_TRUE, 0, sizeof(float) * width * height, image);
}

void CLDCT::execute()
{
	cl_int error;
	cl::Event ev;

	//cl::Kernel twoDimDct = cl::Kernel(program, "twoDimDct");
	//error = twoDimDct.setArg(0, bufferImage);
	//error = twoDimDct.setArg(1, width);
	//error = queue.enqueueNDRangeKernel(twoDimDct, cl::NullRange, cl::NDRange(width / 8, height / 8), cl::NullRange, nullptr, &ev);

    error = queue.enqueueNDRangeKernel(transpose8, cl::NullRange, cl::NDRange(width, height / 8), cl::NDRange(8, 1), nullptr, &ev);
    error = queue.enqueueNDRangeKernel(oneDimDct, cl::NullRange, cl::NDRange(width / 8, height), cl::NullRange, nullptr, &ev);
    error = queue.enqueueNDRangeKernel(transpose8, cl::NullRange, cl::NDRange(width, height / 8), cl::NDRange(8, 1), nullptr, &ev);
    error = queue.enqueueNDRangeKernel(oneDimDct, cl::NullRange, cl::NDRange(width / 8, height), cl::NullRange, nullptr, &ev);
    error = queue.enqueueNDRangeKernel(dctNormalize, cl::NullRange, cl::NDRange(width / 8, height/8), cl::NullRange, nullptr, &ev);

	error = queue.finish();

	error = ev.wait();
}