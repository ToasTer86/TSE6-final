#include <stdio.h>
#include <stdlib.h>
#include <CL/opencl.h>
#include "opencl_utils.h"

#define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x100000)

/*int main() {

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

char string[MEM_SIZE];

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
program = build_program(context, device_id, "./helloWorld.cl");

// create command queue
command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
checkError(ret, "could not create command queue");

// create kernel
kernel = clCreateKernel(program, "test", &ret);
checkError(ret, "could not create kernel");

// create buffer
memobj = clCreateBuffer(context, CL_MEM_READ_WRITE, MEM_SIZE * sizeof(char), NULL, &ret);
checkError(ret, "could not create buffer");

// set kernel args
ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memobj);
checkError(ret, "Could not set kernel args");

// Enqueue ND range kernel
ret = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);
checkError(ret, "Could not enqueue task");

// Enqueue rebuffer
ret = clEnqueueReadBuffer(command_queue, memobj, CL_TRUE, 0,
MEM_SIZE * sizeof(char), string, 0, NULL, NULL);
checkError(ret, "Could not read from buffer");

puts(string);
printf("jajaja");
getchar();
return 0;

}*/