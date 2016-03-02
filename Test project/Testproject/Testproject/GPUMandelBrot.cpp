#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include "bitmap_image.hpp"
#include "mandelbrot_frame.h"
#include <windows.h>
#include <CL/cl.h>
#include "opencl_utils.h"


#define ZOOMFACTOR 400
#define COLORTABLE_SIZE 1024 

mandelbrot_color colortable2[COLORTABLE_SIZE];

unsigned int WIDTH = 800;
unsigned int  HEIGHT = 600;
float OFFSET_X = 0.0;
float OFFSET_Y = 0.0;
unsigned int MAX_ITERATIONS = 1024;

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

void create_colortable()
{
	// Initialize color table values
	for (unsigned int i = 0; i < COLORTABLE_SIZE; i++)
	{
		if (i < 64) {
			mandelbrot_color color_entry = { 0, 0, (5 * i + 20<255) ? 5 * i + 20 : 255 };
			colortable2[i] = color_entry;
		}

		else if (i < 128) {
			mandelbrot_color color_entry = { 0, 2 * i, 255 };
			colortable2[i] = color_entry;
		}

		else if (i < 512) {
			mandelbrot_color color_entry = { 0, (i / 4<255) ? i / 4 : 255, (i / 4<255) ? i / 4 : 255 };
			colortable2[i] = color_entry;
		}

		else if (i < 768) {
			mandelbrot_color color_entry = { 0, (i / 4<255) ? i / 4 : 255, (i / 4<255) ? i / 4 : 255 };
			colortable2[i] = color_entry;
		}

		else {
			mandelbrot_color color_entry = { 0,(i / 10<255) ? i / 10 : 255,(i / 10<255) ? i / 10 : 255 };
			colortable2[i] = color_entry;
		}
	}
}

int main() {

	remove("fractal_output.bmp");

	// Create the colortable and fill it with colors
	create_colortable();

	// Create an empty image
	bitmap_image image(WIDTH, HEIGHT);
	mandelbrot_color * frameBuffer = (mandelbrot_color *)image.data();

	// Get current time before calculating the fractal
	LARGE_INTEGER freq, start;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);

	// Calculate the fractal
	// Get Platform and Device Info //
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	checkError(ret, "could not get platforms");

	// Get device ID // 
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
	checkError(ret, "could not get deviceIDs");

	// Create context with device //
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
	checkError(ret, "could not create context");

	// Build the openCL kernel program
	program = build_program(context, device_id, "./mandelbrot.cl");

	// Create command queue for device
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	checkError(ret, "could not create command queue");

	// Allocate memory on device //
	cl_mem colorTableOnDevice = clCreateBuffer(context, CL_MEM_READ_WRITE, COLORTABLE_SIZE*sizeof(mandelbrot_color), NULL, &ret);
	checkError(ret, "could not allocate memory on device for colortable");

	cl_mem framebufferOnDevice = clCreateBuffer(context, CL_MEM_READ_WRITE, WIDTH*HEIGHT*sizeof(mandelbrot_color), NULL, &ret);
	checkError(ret, "could not allocate memory on device for framebuffer");

	// Build kernel from compiled openCL program //
	kernel = clCreateKernel(program, "mandelbrot_frame", &ret);
	checkError(ret, "could not create kernel");

	// Set kernel arguments //
	ret = clSetKernelArg(kernel, 0, sizeof(float), (void *)&OFFSET_X);
	checkError(ret, "could not set variable 'OFFSET_X'");

	ret = clSetKernelArg(kernel, 1, sizeof(float), (void *)&OFFSET_Y);
	checkError(ret, "could not set variable 'OFFSET_Y'");

	float stepSize = (float)1 / ZOOMFACTOR;
	ret = clSetKernelArg(kernel, 2, sizeof(float), (void *)&stepSize);
	checkError(ret, "could not set variable 'stepSize'");

	ret = clSetKernelArg(kernel, 3, sizeof(unsigned int), (void *)&MAX_ITERATIONS);
	checkError(ret, "could not set variable 'MAX_ITERATIONS'");

	ret = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&framebufferOnDevice);
	checkError(ret, "could not set variable 'framebuffer'");

	ret = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void *)&colorTableOnDevice);
	checkError(ret, "could not set variable 'colortable2'");

	ret = clSetKernelArg(kernel, 6, sizeof(unsigned int), (void *)&WIDTH);
	checkError(ret, "could not set variable 'WIDTH'");

	ret = clSetKernelArg(kernel, 7, sizeof(unsigned int), (void *)&HEIGHT);
	checkError(ret, "could not set variable 'HEIGHT'");

	// Enqueue ND range kernel
	size_t globalSize[] = { WIDTH, HEIGHT }, localSize[] = { 10, 10 };
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
	checkError(ret, "Could not enqueue 2D range on kernel");

	// Enqueue readbuffer
	ret = clEnqueueReadBuffer(command_queue, framebufferOnDevice, CL_TRUE, 0, WIDTH*HEIGHT*sizeof(mandelbrot_color), frameBuffer, 0, NULL, NULL);
	checkError(ret, "Could not request output from device");

	// Get current time after calculating the fractal
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	// Print elapsed time
	printf("Elapsed time to calculate fractal: %f msec\n", (double)(end.QuadPart - start.QuadPart) / freq.QuadPart * 1000.0);
	printf("Press ENTER to continue...\n");
	getchar();

	// Write image to file
	image.save_image("fractal_output.bmp");

	// Show image in mspaint
	WinExec("mspaint fractal_output.bmp", SW_MAXIMIZE);

	return 0;
}