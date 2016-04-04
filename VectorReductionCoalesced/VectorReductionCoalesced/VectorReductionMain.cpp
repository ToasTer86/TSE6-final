#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include <windows.h>
#include <CL/cl.h>
#include "opencl_utils.h"
#include <iostream>

#define _VectorType int
#define VECTOR_SIZE (512*512)
#define COMPUTE_UNITS 512
#define WORKGROUP_SIZE 512

void CreateVector(_VectorType *startingPoint, size_t size)
{
	for (int i = 0; i < size; i++)
	{
		startingPoint[i] = i;
	}
}

int main() {
	//Fractal GPU timing
	cl_event timing_GPU;
	cl_ulong starttime, endtime;

	//Write GPU timing
	cl_event timing_WriteBuffer;
	cl_ulong starttimeWrite, endtimeWrite;

	//Read GPU timing
	cl_event timing_ReadBuffer;
	cl_ulong starttimeRead, endtimeRead;

	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_command_queue command_queue = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_platform_id platform_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret;

	size_t globalSize[]{ VECTOR_SIZE };
	size_t localSize[]{ COMPUTE_UNITS };

	int kernelToRun;

	std::cout << "The array is: " << VECTOR_SIZE << " numbers long." << std::endl;
	std::cout << "Which kernel do you want to run?" << std::endl;
	std::cin >> kernelToRun;

	const char *kernelToUse;

	if (kernelToRun == 1)
	{
		kernelToUse = "./kernel1.cl";
		std::cout << "Okay, using kernel1.cl" << std::endl;
	}
	else if (kernelToRun == 2)
	{
		kernelToUse = "./kernel2.cl";
		std::cout << "Okay, using kernel2.cl" << std::endl;
	}
	else if (kernelToRun == 4)
	{
		kernelToUse = "./kernel4.cl";
		std::cout << "Okay, using kernel4.cl" << std::endl;
	}

	_VectorType *VectorPtr = new _VectorType[VECTOR_SIZE];
	_VectorType *VectorPtrFromGpu = new _VectorType[VECTOR_SIZE];

	CreateVector(VectorPtr, VECTOR_SIZE);

	// Get current time before reducing the vector//
	LARGE_INTEGER freq, start;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);

	_VectorType cpuSum = 0;
	for (int i = 0; i < VECTOR_SIZE; i++)
	{
		cpuSum += VectorPtr[i];
	}

	// TIMING CPU
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	// PRINT TIME CPU
	printf("Elapsed time to reduce vector with CPU: %f msec\n", (double)(end.QuadPart - start.QuadPart) / freq.QuadPart * 1000.0);

	// Get Platform and Device Info //
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	checkError(ret, "could not get platforms");

	// Get device ID // 
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
	checkError(ret, "could not get deviceIDs");

	// Create context with device //
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
	checkError(ret, "could not create context");

	// Build the openCL kernel program //
	program = build_program(context, device_id, kernelToUse);

	// Create command queue for device //
	command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret);
	checkError(ret, "could not create command queue");

	// Allocate memory on device //
	cl_mem vectorOnDevice = clCreateBuffer(context, CL_MEM_READ_WRITE, VECTOR_SIZE*sizeof(_VectorType), NULL, &ret);
	checkError(ret, "could not allocate memory on device for vector");

	// Write vector to device //
	ret = clEnqueueWriteBuffer(command_queue, vectorOnDevice, CL_TRUE, 0, VECTOR_SIZE*sizeof(_VectorType), VectorPtr, 0, NULL, &timing_WriteBuffer);
	checkError(ret, "Could not write vector to device");

	// Build kernel from compiled openCL program //
	kernel = clCreateKernel(program, "reduction", &ret);
	checkError(ret, "could not create kernel");

	// Set kernel arguments //
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&vectorOnDevice);
	checkError(ret, "could not set kernel arg 0");

	ret = clSetKernelArg(kernel, 1, (VECTOR_SIZE / COMPUTE_UNITS)*sizeof(int), NULL);
	checkError(ret, "could not set kernel arg 1");

	// Enqueue ND range kernel
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, globalSize, localSize, 0, NULL, &timing_GPU);
	checkError(ret, "Could not enqueue 1D range on kernel");

	// TIMING GPU
	clFinish(command_queue);
	clGetEventProfilingInfo(timing_GPU, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &starttime, NULL);
	clGetEventProfilingInfo(timing_GPU, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endtime, NULL);

	float totalKernelTime = (float)(endtime - starttime);

	int amtOfResults = VECTOR_SIZE / WORKGROUP_SIZE;
	printf("remaining amount of results: %i\n", amtOfResults);

	int iterations = 1;

	while (amtOfResults != 1)
	{
		int nextWorkGroupSize = pow(WORKGROUP_SIZE, iterations);
		int _globalSize = VECTOR_SIZE / nextWorkGroupSize;
		globalSize[0] = _globalSize;

		// Enqueue ND range kernel
		ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, globalSize, localSize, 0, NULL, &timing_GPU);
		checkError(ret, "Could not enqueue 1D range on kernel");

		amtOfResults = _globalSize / WORKGROUP_SIZE;
		printf("remaining amount of results: %i\n", amtOfResults);
		iterations++;

		clFinish(command_queue);
		clGetEventProfilingInfo(timing_GPU, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &starttime, NULL);
		clGetEventProfilingInfo(timing_GPU, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endtime, NULL);
		totalKernelTime += (float)(endtime - starttime);
	}
	// Enqueue readbuffer
	ret = clEnqueueReadBuffer(command_queue, vectorOnDevice, CL_TRUE, 0, VECTOR_SIZE*sizeof(_VectorType), VectorPtrFromGpu, 0, NULL, &timing_ReadBuffer);
	checkError(ret, "Could not request output from device");

	//TIMING READ & WRITE BUFFER
	clGetEventProfilingInfo(timing_WriteBuffer, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &starttimeWrite, NULL);
	clGetEventProfilingInfo(timing_WriteBuffer, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endtimeWrite, NULL);
	clGetEventProfilingInfo(timing_ReadBuffer, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &starttimeRead, NULL);
	clGetEventProfilingInfo(timing_ReadBuffer, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endtimeRead, NULL);

	//PRINT GPU TIME
	printf("Kernel execution took %f msec\n", totalKernelTime / 1000000);
	printf("Writing of buffer took %f msec\n", (float)(endtimeWrite - starttimeWrite) / 1000000);
	printf("Reading of buffer took %f msec\n", (float)(endtimeRead - starttimeRead) / 1000000);
	printf("Result from GPU: %i\n", VectorPtrFromGpu[0]);
	printf("Result from CPU: %i\n", cpuSum);

	system("pause");

	char rerun;

	std::cout << "Do you want to run the program again? Y or N \n";
	std::cin >> rerun;

	free(VectorPtr);
	free(VectorPtrFromGpu);

	clReleaseMemObject(vectorOnDevice);
	

	if (rerun == 'Y')
	{
		return main();
	}
	else
	{
		return 0;
	}
}