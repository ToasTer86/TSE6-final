#include <stdio.h>
#include <stdlib.h>
#include <CL/opencl.h>
#include "opencl_utils.h"

#define MAX_SOURCE_SIZE (0x100000)

int main() {

	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_command_queue command_queue = NULL;
	cl_mem memobj = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_platform_id platform_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret;

	/* Arrays */
	float arrayA[1000];
	float arrayB[1000];

	for (int i = 0; i < 1000; i++)
	{
		arrayA[i] = i;
		arrayB[i] = i;
	}

	float arrayC[1000];

	// Get Platform and Device Info //
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	checkError(ret, "could not get platforms");

	// Get device ID // 
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
	checkError(ret, "could not get deviceID's");

	// create context with device //
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
	checkError(ret, "could not create context");

	// build program
	program = build_program(context, device_id, "./arrayAddition.cl");

	// create command queue
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	checkError(ret, "could not create command queue");

	/* Allocate memory on device */
	cl_mem AonDevice = clCreateBuffer(context, CL_MEM_READ_WRITE, 1000 * sizeof(float), NULL, &ret);
	cl_mem BonDevice = clCreateBuffer(context, CL_MEM_READ_WRITE, 1000 * sizeof(float), NULL, &ret);
	cl_mem ConDevice = clCreateBuffer(context, CL_MEM_READ_WRITE, 1000 * sizeof(float), NULL, &ret);

	/* Write arrays to allocated device memory */
	ret = clEnqueueWriteBuffer(command_queue, AonDevice, CL_TRUE, 0, 1000 * sizeof(float), arrayA, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, BonDevice, CL_TRUE, 0, 1000 * sizeof(float), arrayB, 0, NULL, NULL);

	// create kernel
	kernel = clCreateKernel(program, "addArray", &ret);
	checkError(ret, "could not create kernel");

	/* Set kernel arguments */
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&AonDevice);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&BonDevice);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&ConDevice);

	// Enqueue ND range kernel
	size_t globalSize[] = { 100, 10 }, localSize[] = { 1, 1 };
	ret = clEnqueueNDRangeKernel
		(command_queue,
			kernel,
			2,
			NULL,
			globalSize,
			localSize,
			0,
			NULL,
			NULL
			);

	// Enqueue readbuffer
	ret = clEnqueueReadBuffer(command_queue, ConDevice, CL_TRUE, 0, 1000 * sizeof(float), arrayC, 0, NULL, NULL);

	for (int i = 0; i < 1000; i++)
	{
		printf("array item %u: %f\n", i, arrayC[i]);
	}

	printf("jajaja");
	getchar();
	return 0;

}