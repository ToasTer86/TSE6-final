/*
* Adventures in OpenCL tutorial series
* Part 2
*
* author: Ian Johnson
* htt://enja.org
* code based on advisor Gordon Erlebacher's work
* NVIDIA's examples
* as well as various blogs and resources on the internet
*/
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <math.h>
#include "cpu_particle.h"

//OpenGL stuff
#include <GL/glew.h>
#if defined __APPLE__ || defined(MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

//Our OpenCL Particle Systemclass
#include "cll.h"
#include <iostream>
CL* example;

//GL related variables
int window_width = 800;
int window_height = 600;
int glutWindowHandle = 0;
float translate_z = -1.f;
// mouse controls
int mouse_old_x, mouse_old_y;
int mouse_buttons = 0;
float rotate_x = 0.0, rotate_y = 0.0;
//main app helper functions
void init_gl(int argc, char** argv);
void appRender();
int appDestroy();
void timerCB(int ms);
void appKeyboard(unsigned char key, int x, int y);
void appMouse(int button, int state, int x, int y);
void appMotion(int x, int y);

float r = rand() / (float)RAND_MAX;
float g = rand() / (float)RAND_MAX;
float b = rand() / (float)RAND_MAX;

std::vector<Vec4> color();

char GPU_CPU_choice = NULL;
int workgroupsize = NULL;
double particles = NULL;

float dt = .01f;

void menuParticles(int value) {
	switch (value) {
	case 1:
		particles = particles + 1024;
		break;
	case 2:
		particles = particles - 1024;
		break;
	case 3:
		r = 1.0;
		g = 0.0f;
		b = 0.0f;
		break;
	case 4:
		r = 0.0f;
		g = 0.0f;
		b = 1.0f;
		break;
	case 5:
		r = 0.0f;
		g = 1.0f;
		b = 0.0f;
		break;

	}
}


//----------------------------------------------------------------------
//quick random function to distribute our initial points
float rand_float(float mn, float mx)
{
	float r = rand() / (float)RAND_MAX;
	return mn + (mx - mn)*r;
}

std::string getKernelSource(const char *filename)
{
	FILE *program_handle;
	char *program_buffer, *program_log;
	size_t program_size, log_size;
	cl_int err;

	program_handle = fopen(filename, "rb");
	if (program_handle == NULL) {
		perror("Couldn't find the program file");
		getchar(); exit(1);
	}

	fseek(program_handle, 0, SEEK_END);
	program_size = ftell(program_handle);
	rewind(program_handle);
	program_buffer = (char*)malloc(program_size + 1);
	program_buffer[program_size] = '\0';
	fread(program_buffer, sizeof(char), program_size, program_handle);
	fclose(program_handle);

	return std::string(program_buffer);
}


//----------------------------------------------------------------------
int main(int argc, char** argv)
{
	std::cout << "Run program with CPU or GPU?" << std::endl;
	std::cout << "G for GPU / C for CPU" << std::endl;
	std::cin >> GPU_CPU_choice;

	std::cout << "Configure the workgroupsize (multitudes of 8)" << std::endl;
	std::cin >> workgroupsize;

	std::cout << "Set the amount of particles (multitudes of 8)" << std::endl;
	std::cin >> particles;

	printf("Hello, OpenCL\n");
	//Setup our GLUT window and OpenGL related things
	//glut callback functions are setup here too

	//initialize our particle system with positions, velocities and color
	int num = particles;
	std::vector<Vec4> pos(num);
	std::vector<Vec4> vel(num);
	std::vector<Vec4> color(num);

	//fill our vectors with initial data
	for (int i = 0; i < num; i++)
	{
		//distribute the particles in a random circle around z axis
		float rad = rand_float(0.2, 0.6);
		float x = rad*sin(2 * 3.14 * i / num);
		float z = 0.0f;// -.1 + .2f * i/num;
		float y = rad*cos(2 * 3.14 * i / num);
		pos[i] = Vec4(x, y, z, 1.0f);

		//give some initial velocity 
		//float xr = rand_float(-.1, .1);
		//float yr = rand_float(1.f, 3.f);
		//the life is the lifetime of the particle: 1 = alive 0 = dead
		//as you will see in part2.cl we reset the particle when it dies
		float life_r = rand_float(0.f, 1.f);
		vel[i] = Vec4(1.0, 2.0, 3.0f, life_r);


		//just make them red and full alpha
		float r = 1.0f; //rand() / (float)RAND_MAX;
		float g = 0.0f; //rand() / (float)RAND_MAX;
		float b = 0.0f; // rand() / (float)RAND_MAX; 
		color[i] = Vec4(r, g, b, 1.0f);
	}

	if (GPU_CPU_choice == 'G')
	{
		init_gl(argc, argv);

		//initialize our CL object, this sets up the context
		example = new CL();
		//load and build our CL program from the file
		std::string kernel_source;
		kernel_source = getKernelSource("./particle2.cl");
		example->loadProgram(kernel_source);
		//our load data function sends our initial values to the GPU
		example->loadData(pos, vel, color);
		//initialize the kernel
		example->popCorn();
		//this starts the GLUT program, from here on out everything we want
		//to do needs to be done in glut callback functions
		glutMainLoop();
	}
	else if(GPU_CPU_choice == 'C')
	{
		// Get current time before calculating the fractal
		LARGE_INTEGER freq, start;
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&start);

		for (int i = 0; i < particles - 1; i++)
		{
			cpuKernel(&pos[0], &color[0], &vel[0], dt, i);
		}
		// Get current time after calculating the fractal
		LARGE_INTEGER end;
		QueryPerformanceCounter(&end);

		// Print elapsed time
		printf("Elapsed time to calculate all pixels: %f msec\n", (double)(end.QuadPart - start.QuadPart) / freq.QuadPart * 1000.0);

		system("pause");
		return main(1, NULL);
	}

}


//----------------------------------------------------------------------
void appRender()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//this updates the particle system by calling the kernel
	example->runKernel(workgroupsize, particles);

	//render the particles from VBOs
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(5.);

	//printf("color buffer\n");
	glBindBuffer(GL_ARRAY_BUFFER, example->c_vbo);
	glColorPointer(4, GL_FLOAT, 0, 0);

	//printf("vertex buffer\n");
	glBindBuffer(GL_ARRAY_BUFFER, example->p_vbo);
	glVertexPointer(4, GL_FLOAT, 0, 0);

	//printf("enable client state\n");
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	//Need to disable these for blender
	glDisableClientState(GL_NORMAL_ARRAY);

	//printf("draw arrays\n");
	glDrawArrays(GL_POINTS, 0, particles);

	//printf("disable stuff\n");
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glutSwapBuffers();
}


//----------------------------------------------------------------------
void init_gl(int argc, char** argv)
{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(window_width, window_height);
	glutInitWindowPosition(glutGet(GLUT_SCREEN_WIDTH) / 2 - window_width / 2,
		glutGet(GLUT_SCREEN_HEIGHT) / 2 - window_height / 2);


	std::stringstream ss;
	ss << "Particle game of life with " << particles << " particles" << std::ends;
	glutWindowHandle = glutCreateWindow(ss.str().c_str());

	glutDisplayFunc(appRender); //main rendering function
	glutTimerFunc(30, timerCB, 30); //determin a minimum time between frames
	glutKeyboardFunc(appKeyboard);
	glutMouseFunc(appMouse);
	glutMotionFunc(appMotion);

	glewInit();

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glDisable(GL_DEPTH_TEST);

	// viewport
	glViewport(0, 0, window_width, window_height);

	// projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, (GLfloat)window_width / (GLfloat)window_height, 0.1, 1000.0);

	glutCreateMenu(menuParticles);
	glutAddMenuEntry(" 1024 More particles", 1);
	glutAddMenuEntry(" 1024 Less particles", 2);
	glutAddMenuEntry("Red", 3);
	glutAddMenuEntry("Blue", 4);
	glutAddMenuEntry("Green", 5);

	// set view matrix
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, translate_z);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}


//----------------------------------------------------------------------
int appDestroy()
{
	//this makes sure we properly cleanup our OpenCL context
	delete example;
	if (glutWindowHandle)glutDestroyWindow(glutWindowHandle);
	printf("about to exit!\n");

	exit(0);
}


//----------------------------------------------------------------------
void timerCB(int ms)
{
	//this makes sure the appRender function is called every ms miliseconds
	glutTimerFunc(ms, timerCB, ms);
	glutPostRedisplay();
}


//----------------------------------------------------------------------
void appKeyboard(unsigned char key, int x, int y)
{
	//this way we can exit the program cleanly
	switch (key)
	{
	case '\033': // escape quits
	case '\015': // Enter quits    
	case 'Q':    // Q quits
	case 'q':    // q (or escape) quits
				 // Cleanup up and quit
	case 'r': 
		appDestroy();
		break;
	}
}


//----------------------------------------------------------------------
void appMouse(int button, int state, int x, int y)
{
	//handle mouse interaction for rotating/zooming the view
	if (state == GLUT_DOWN) {
		mouse_buttons |= 1 << button;
	}
	else if (state == GLUT_UP) {
		mouse_buttons = 0;
	}

	mouse_old_x = x;
	mouse_old_y = y;
}


//----------------------------------------------------------------------
void appMotion(int x, int y)
{
	//hanlde the mouse motion for zooming and rotating the view
	float dx, dy;
	dx = x - mouse_old_x;
	dy = y - mouse_old_y;

	if (mouse_buttons & 1) {
		rotate_x += dy * 0.2;
		rotate_y += dx * 0.2;
	}
	else if (mouse_buttons & 4) {
		translate_z += dy * 0.1;
	}

	mouse_old_x = x;
	mouse_old_y = y;

	// set view matrix
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, translate_z);
	glRotatef(rotate_x, 1.0, 0.0, 0.0);
	glRotatef(rotate_y, 0.0, 1.0, 0.0);
}



