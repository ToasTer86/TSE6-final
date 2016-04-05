#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include "bitmap_image.hpp"
#include "mandelbrot_frame.h"
#include <windows.h>
#include <CL/cl.h>
#include "opencl_utils.h"
#include "color_table.h"
#include "OpenGL_functions.h"
#include "include/GL/freeglut.h"

#define COLORTABLE_SIZE 2048 

mandelbrot_color colorTable[COLORTABLE_SIZE];

unsigned int WIDTH = 1920;
unsigned int  HEIGHT = 1080;
unsigned int ZOOMFACTOR = 100;
float OFFSET_X = -0.251995001;
float OFFSET_Y = 0.0001429999;
unsigned int MAX_ITERATIONS = 2048;

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

int deltaTime;
float stepSize = (float)1 / ZOOMFACTOR;
float ZOOMSPEED = 0.98;

cl_mem cl_tex;
GLuint texture;

int now = 0;
int previous = 0;

size_t localSize[] = { 10, 10 };

void getZoomSpeed(unsigned char key, int mouseX, int mouseY)
{
	switch (key)
	{
	case '-':
			ZOOMSPEED += 0.01;
			break;

	case '=':
			ZOOMSPEED -= 0.01;
			break;
	}

}

void CreateColortable()
{
	// Initialize color table values
	for (unsigned int i = 0; i < COLORTABLE_SIZE; i++)
	{
		if (i < 64)
		{
			mandelbrot_color color_entry = { 0, 0, (5 * i + 20 < 255) ? 5 * i + 20 : 255 };
			colorTable[i] = color_entry;
		}

		else if (i < 128) //128
		{
			mandelbrot_color color_entry = { 0, 2 * i, 255 };
			colorTable[i] = color_entry;
		}

		else if (i < 512) //512
		{
			mandelbrot_color color_entry = { 0, (i / 4 < 255) ? i / 4 : 255, (i / 4 < 255) ? i / 4 : 255 };
			colorTable[i] = color_entry;
		}

		else if (i < 768) //768
		{
			mandelbrot_color color_entry = { 0, (i / 4 < 255) ? i / 4 : 255, (i / 4 < 255) ? i / 4 : 255 };
			colorTable[i] = color_entry;
		}

		else
		{
			mandelbrot_color color_entry = { 0,(i / 10 < 255) ? i / 10 : 255,(i / 10 < 255) ? i / 10 : 255 };
			colorTable[i] = color_entry;
		}
	}
}

int oldTimeSinceStart = 0;

void display()
{
	glFinish();

	clEnqueueAcquireGLObjects(command_queue, 1, &cl_tex, 0, NULL, NULL);
	size_t globalSize[] = { WIDTH, HEIGHT };
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, globalSize, localSize, 0, NULL, NULL);
	checkError(ret, "Could not enqueue 2D range on kernel");

	clEnqueueReleaseGLObjects(command_queue, 1, &cl_tex, 0, NULL, NULL);

	now = glutGet(GLUT_ELAPSED_TIME);
	stepSize *= pow(ZOOMSPEED, (now - previous) / 100.0);
	previous = now;

	ret = clSetKernelArg(kernel, 2, sizeof(float), &stepSize);
	checkError(ret, "could not set variable 'stepSize'");

	clFinish(command_queue);

	draw_quad();

	glFlush();

	glutPostRedisplay(); // Necessary if scene varies over time (animation)
}

int main(int argc, char** argv){

	glutInit(&argc, argv);					// Initialize GLUT
	glutInitWindowSize(WIDTH, HEIGHT);		// Define window size
	glutCreateWindow("harry");				// Create window with title
	glutDisplayFunc(display);				// When the window has to be                              
											// redrawn, then function                               
											// display will be called 

	create_colortable(colorTable, COLORTABLE_SIZE);

	init_gl(800, 600);
	// Create the colortable and fill it with colors
	CreateColortable();

	// Create an empty image //
	bitmap_image image(WIDTH, HEIGHT);
	mandelbrot_color * frameBuffer = (mandelbrot_color *)image.data();

	// Get Platform and Device Info //
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	checkError(ret, "could not get platforms");

	// Get device ID // 
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
	checkError(ret, "could not get deviceIDs");

	// Create context with device //
	cl_context_properties properties[] = { CL_GL_CONTEXT_KHR,         reinterpret_cast<cl_context_properties>(wglGetCurrentContext()),        CL_WGL_HDC_KHR,         reinterpret_cast<cl_context_properties>(wglGetCurrentDC()),        0 };

	context = clCreateContext(properties, 1, &device_id, NULL, NULL, &ret);
	checkError(ret, "could not create context");

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glEnable(GL_TEXTURE_2D);

	cl_tex = clCreateFromGLTexture2D(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, texture, &ret);
	checkError(ret, "could not create CL / GL texture");

	// Build the openCL kernel program //
	program = build_program(context, device_id, "./mandelbrot.cl");

	// Create command queue for device //
	command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret);
	checkError(ret, "could not create command queue");

	// Allocate memory on device //
	cl_mem colorTableOnDevice = clCreateBuffer(context, CL_MEM_READ_WRITE, COLORTABLE_SIZE*sizeof(mandelbrot_color), NULL, &ret);
	checkError(ret, "could not allocate memory on device for colortable");

	cl_mem framebufferOnDevice = clCreateBuffer(context, CL_MEM_READ_WRITE, WIDTH*HEIGHT*sizeof(mandelbrot_color), NULL, &ret);
	checkError(ret, "could not allocate memory on device for framebuffer");

	// write buffer //
	ret = clEnqueueWriteBuffer(command_queue, colorTableOnDevice, CL_TRUE, 0, COLORTABLE_SIZE* sizeof(mandelbrot_color), colorTable, 0, NULL, NULL);

	// Build kernel from compiled openCL program //
	kernel = clCreateKernel(program, "mandelbrot_frame", &ret);
	checkError(ret, "could not create kernel");

	// Enqueue ND range kernel
	size_t globalSize[] = { WIDTH, HEIGHT };
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, globalSize, localSize, 0, NULL, NULL);
	checkError(ret, "Could not enqueue 2D range on kernel");

	// Set kernel arguments //
	ret = clSetKernelArg(kernel, 0, sizeof(float), &OFFSET_X);
	checkError(ret, "could not set variable 'OFFSET_X'");

	ret = clSetKernelArg(kernel, 1, sizeof(float), &OFFSET_Y);
	checkError(ret, "could not set variable 'OFFSET_Y'");

	ret = clSetKernelArg(kernel, 2, sizeof(float), &stepSize);
	checkError(ret, "could not set variable 'stepSize'"); 

	ret = clSetKernelArg(kernel, 3, sizeof(unsigned int), &MAX_ITERATIONS);
	checkError(ret, "could not set variable 'MAX_ITERATIONS'");

	ret = clSetKernelArg(kernel, 4, sizeof(cl_tex), (void*)&cl_tex);
	checkError(ret, "could not set variable 'cl_tex'");

	ret = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*)&colorTableOnDevice);
	checkError(ret, "could not set variable 'colortable2'");

	ret = clSetKernelArg(kernel, 6, sizeof(unsigned int), &WIDTH);
	checkError(ret, "could not set variable 'WIDTH'");

	ret = clSetKernelArg(kernel, 7, sizeof(unsigned int), &HEIGHT);
	checkError(ret, "could not set variable 'HEIGHT'");

	// Enqueue readbuffer
	ret = clEnqueueReadBuffer(command_queue, framebufferOnDevice, CL_TRUE, 0, WIDTH*HEIGHT*sizeof(mandelbrot_color), frameBuffer, 0, NULL, NULL);
	checkError(ret, "Could not request output from device");

	glutKeyboardFunc(getZoomSpeed);
	glutMainLoop();

	return 0;

}