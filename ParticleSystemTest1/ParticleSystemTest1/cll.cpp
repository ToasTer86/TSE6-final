#include <stdio.h>
#include <string>
#include <iostream>

#include "cll.h"
#include "util.h"
#include <Windows.h>

CL::CL()
{
	printf("Initialize OpenCL object and context\n");
	//setup devices and context
	std::vector<cl::Platform> platforms;
	err = cl::Platform::get(&platforms);
	printf("cl::Platform::get(): %s\n", oclErrorString(err));
	printf("platforms.size(): %d\n", platforms.size());

	deviceUsed = 0;
	err = platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
	printf("getDevices: %s\n", oclErrorString(err));
	printf("devices.size(): %d\n", devices.size());
	int t = devices.front().getInfo<CL_DEVICE_TYPE>();
	printf("type: device: %d CL_DEVICE_TYPE_GPU: %d \n", t, CL_DEVICE_TYPE_GPU);

	cl_context_properties props[] =
	{
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(),
		0
	};
	//cl_context cxGPUContext = clCreateContext(props, 1, &cdDevices[uiDeviceUsed], NULL, NULL, &err);
	try {
		context = cl::Context(CL_DEVICE_TYPE_GPU, props);
	}
	catch (cl::Error er) {
		printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
	}

	//create the command queue we will use to execute OpenCL commands
	try {
		queue = cl::CommandQueue(context, devices[deviceUsed], CL_QUEUE_PROFILING_ENABLE, &err);
	}
	catch (cl::Error er) {
		printf("ERROR: %s(%d)\n", er.what(), er.err());
	}

}

CL::~CL()
{
}


void CL::loadProgram(std::string kernel_source)
{
	// Program Setup
	int pl;
	//size_t program_length;
	printf("load the program\n");

	pl = kernel_source.size();
	printf("kernel size: %d\n", pl);
	//printf("kernel: \n %s\n", kernel_source.c_str());
	try
	{
		cl::Program::Sources source(1,
			std::make_pair(kernel_source.c_str(), pl));
		program = cl::Program(context, source);
	}
	catch (cl::Error er) {
		printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
	}

	printf("build program\n");
	try
	{
		//err = program.build(devices, "-cl-nv-verbose -cl-nv-maxrregcount=100");
		err = program.build(devices);
	}
	catch (cl::Error er) {
		printf("program.build: %s\n", oclErrorString(er.err()));
		//if(err != CL_SUCCESS){
	}
	printf("done building program\n");
	std::cout << "Build Status: " << program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(devices[0]) << std::endl;
	std::cout << "Build Options:\t" << program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(devices[0]) << std::endl;
	std::cout << "Build Log:\t " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;

}

