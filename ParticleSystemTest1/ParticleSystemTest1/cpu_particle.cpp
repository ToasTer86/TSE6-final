#include <stdio.h>
#include "cll.h"
#include "util.h"
#include <vector>

void cpuKernel(Vec4* pos, Vec4* color, Vec4* vel, float dt, int i)
{
		//copy position and velocity for this iteration to a local variable
		//note: if we were doing many more calculations we would want to have opencl
		//copy to a local memory array to speed up memory access (this will be the subject of a later tutorial)
		Vec4 p = pos[i];
		Vec4 v = vel[i];

		//we've stored the life in the fourth component of our velocity array
		float life = vel[i].w;
		//decrease the life by the time step (this value could be adjusted to lengthen or shorten particle life
		life -= dt;
		//if the life is 0 or less we reset the particle's values back to the original values and set life to 1
		if (life <= 0)
		{
			p = pos[i];
			v = vel[i];
			life = 1.0;
		}

		//we use a first order euler method to integrate the velocity and position (i'll expand on this in another tutorial)
		//update the velocity to be affected by "gravity" in the z direction
		v.z -= 9.8*dt;
		//update the position with the new velocity
		p.z += v.z*dt;
		//store the updated life in the velocity array
		v.w = life;

		//update the arrays with our newly computed values
		pos[i] = p;
		vel[i] = v;

		//you can manipulate the color based on properties of the system
		//here we adjust the alpha

		color[i].w = life;
	}
