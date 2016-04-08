#include <stdio.h>
#include "cll.h"
#include "util.h"
#include <vector>
#include <ctime>

void CL::loadData(std::vector<Vec4> pos, std::vector<Vec4> vel, std::vector<Vec4> col)
{
	//store the number of particles and the size in bytes of our arrays
	array_size = pos.size() * sizeof(Vec4);
	//create VBOs (defined in util.cpp)
	p_vbo = createVBO(&pos[0], array_size, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
	c_vbo = createVBO(&col[0], array_size, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);

	//make sure OpenGL is finished before we proceed
	glFinish();
	printf("gl interop!\n");
	// create OpenCL buffer from GL VBO
	cl_vbos.push_back(cl::BufferGL(context, CL_MEM_READ_WRITE, p_vbo, &err));
	//printf("v_vbo: %s\n", oclErrorString(err));
	cl_vbos.push_back(cl::BufferGL(context, CL_MEM_READ_WRITE, c_vbo, &err));
	//we don't need to push any data here because it's already in the VBO


	//create the OpenCL only arrays
	cl_velocities = cl::Buffer(context, CL_MEM_WRITE_ONLY, array_size, NULL, &err);
	cl_pos_gen = cl::Buffer(context, CL_MEM_WRITE_ONLY, array_size, NULL, &err);
	cl_vel_gen = cl::Buffer(context, CL_MEM_WRITE_ONLY, array_size, NULL, &err);

	printf("Pushing data to the GPU\n");
	//push our CPU arrays to the GPU
	//data is tightly packed in std::vector starting with the adress of the first element
	err = queue.enqueueWriteBuffer(cl_velocities, CL_TRUE, 0, array_size, &vel[0], NULL, &event);
	err = queue.enqueueWriteBuffer(cl_pos_gen, CL_TRUE, 0, array_size, &pos[0], NULL, &event);
	err = queue.enqueueWriteBuffer(cl_vel_gen, CL_TRUE, 0, array_size, &vel[0], NULL, &event);
	queue.finish();
}

void CL::popCorn()
{
	printf("in popCorn\n");
	//initialize our kernel from the program
	try {
		kernel = cl::Kernel(program, "particle", &err);
	}
	catch (cl::Error er) {
		printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
	}

	//set the arguements of our kernel
	try
	{
		err = kernel.setArg(0, cl_vbos[0]); //position vbo
		err = kernel.setArg(1, cl_vbos[1]); //color vbo
		err = kernel.setArg(2, cl_velocities);
		err = kernel.setArg(3, cl_pos_gen);
		err = kernel.setArg(4, cl_vel_gen);
	}
	catch (cl::Error er) {
		printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
	}
	//Wait for the command queue to finish these commands before proceeding
	queue.finish();
}



void CL::runKernel(int workgroupsize, double particles)
{
	// we will run the program for 10 seconds
	std::clock_t start;
	double duration = 0;
	long long amountOfExecutions = 0; 
	float average_executiontime = 0;
	long long i = 0;
	float executiontime[1000];

	start = clock();
	duration = (clock() - start) / (double)CLOCKS_PER_SEC;
	printf("duration %F:", duration);

	//this will update our system by calculating new velocity and updating the positions of our particles
	//Make sure OpenGL is done using our VBOs
	glFinish();
	// map OpenGL buffer object for writing from OpenCL
	//this passes in the vector of VBO buffer objects (position and color)
	err = queue.enqueueAcquireGLObjects(&cl_vbos, NULL, &event);
	//printf("acquire: %s\n", oclErrorString(err));
	queue.finish();

	float dt = .01f;
	kernel.setArg(5, dt); //pass in the timestep
						  //execute the kernel
	cl::Event kernelExecution;
	err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(particles), cl::NDRange(workgroupsize), NULL, &kernelExecution);
	//printf("clEnqueueNDRangeKernel: %s\n", oclErrorString(err));
	kernelExecution.wait();
	queue.finish();

	amountOfExecutions++;
	executiontime[amountOfExecutions] = (float)(kernelExecution.getProfilingInfo<CL_PROFILING_COMMAND_END>() - kernelExecution.getProfilingInfo<CL_PROFILING_COMMAND_START>());
	//Release the VBOs so OpenGL can play with them
	err = queue.enqueueReleaseGLObjects(&cl_vbos, NULL, &event);
	//printf("release gl: %s\n", oclErrorString(err));
	queue.finish();

		for (i < amountOfExecutions; i++;)
		{
			average_executiontime += executiontime[i];
 		}
		average_executiontime = average_executiontime / amountOfExecutions;
		printf("In the last 10 seconds the average kernel execution time was: %f msec", average_executiontime);
		i = 0;
		duration = 0;
}