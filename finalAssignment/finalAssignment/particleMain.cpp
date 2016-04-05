#include <windows.h>
#include <CL/cl.h>
#include "opencl_utils.h"
#include "OpenGL_functions.h"
#include "GL/freeglut.h"

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

void openCL_startup()
{
	// Get Platform and Device Info //
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	checkError(ret, "could not get platforms");

	// Get device ID // 
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
	checkError(ret, "could not get deviceIDs");
}

void openCL_create_buffer()
{
	
}

void openCL_create_program_kernel()
{

}

int main ()
{
	return 0;
}