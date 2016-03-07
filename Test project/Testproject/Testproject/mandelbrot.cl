//#include "mandelbrot_frame.h"

typedef struct {
	unsigned char blue, green, red;
} mandelbrot_color; 

__kernel void mandelbrot_frame(
	const float x0,
	const float y0,
	const float stepsize,
	const unsigned int max_iterations,
	__global mandelbrot_color *framebuffer,
	__global const mandelbrot_color *colortable,
	const unsigned int window_width,
	const unsigned int window_height)
{	
	float windowPosX = (float)get_global_id(0);
	float windowPosY = (float)get_global_id(1);

	float center_X = -(stepsize*window_width / 2);
	float center_Y = (stepsize*window_height / 2);
	const float stepPosX = center_X - x0 + (windowPosX * stepsize);
	const float stepPosY = center_Y + y0 - (windowPosY * stepsize);

	// Variables for the calculation
	float x = 0.0;
	float y = 0.0;
	float xSqr = 0.0;
	float ySqr = 0.0;
	unsigned int iterations = 0;

	// Perform up to the maximum number of iterations to solve
	// the current pixel's position in the image
	while ((xSqr + ySqr < 4.0) && (iterations < max_iterations))
	{
		// Perform the current iteration
		xSqr = x*x;
		ySqr = y*y;

		y = 2 * x*y + stepPosY;
		x = xSqr - ySqr + stepPosX;

		// Increment iteration count
		iterations++;
	}

	// Output black if we never finished, and a color from the look up table otherwise
	mandelbrot_color black = { 0,0,0 };
	int arrayIndex = (int)(window_width * windowPosY + windowPosX);
	framebuffer[arrayIndex] = (iterations == max_iterations) ? black : colortable[iterations];
	//printf("framebuffer[%u]: %f\n", arrayIndex, framebuffer[arrayIndex]);
}