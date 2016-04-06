__kernel void particle2(__global float4* pos, __global float4* color, __global float4* vel, __global float4* pos_gen, __global float4* vel_gen, float dt)
{
	//get our index in the array
	unsigned int i = get_global_id(0);

	//decrease the life by the time step (this value could be adjusted to lengthen or shorten particle life
	vel[i].w -= dt;
	//if the life is 0 or less we reset the particle's values back to the original values and set life to 1
	if (vel[i].w <= 0)
	{
		pos[i] = pos_gen[i];
		vel[i] = vel_gen[i];
		vel[i].w = 1.0;
	}

	//we use a first order euler method to integrate the velocity and position (i'll expand on this in another tutorial)
	//update the velocity to be affected by "gravity" in the z direction
	vel.z -= 9.8*dt;
	//update the position with the new velocity
	*pos.z += vel.z*dt;
	*pos.y += vel.y*dt;
	//p.z += v.z*dt;
	//store the updated life in the velocity array
	vel.w = vel[i].w;

	//you can manipulate the color based on properties of the system
	//here we adjust the alpha
	if (i == 1) {
		color[i].x = 1;
		color[i].y = 1;
		color[i].z = 1;
		color[i].w = 1;
	}
	else
	{
		color[i].w = vel[i].w;
	}
}