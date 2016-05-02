__kernel void particle(__global float4* pos, __global float4* color, __global float4* vel, __global float4* pos_gen, __global float4* vel_gen, float dt)
{
	//get our index in the array
	unsigned int i = get_global_id(0);
	unsigned int i_secondary = get_global_size(0) + i;

	//copy position and velocity for this iteration to a local variable
	float4 p = pos[i];
	float4 p2 = pos[i_secondary];
	float4 v = vel[i];
	float4 v2 = vel[i_secondary];

	//we've stored the life in the fourth component of our velocity array
	float life = vel[i].w;
	float life2 = vel[i_secondary].w;
	//decrease the life by the time step (this value could be adjusted to lengthen or shorten particle life
	life -= dt;
	life2 -= dt;
	//if the life is 0 or less we reset the particle's values back to the original values and set life to 1
	if (life <= 0)
	{
		p = pos_gen[i];
		v = vel_gen[i];
		life = 1.0;
	}
	if (life2 <= 0)
	{
		p2 = pos_gen[i_secondary];
		v2 = vel_gen[i_secondary];
		life2 = 1.0;
	}

	//we use a first order euler method to integrate the velocity and position (i'll expand on this in another tutorial)
	//update the velocity to be affected by "gravity" in the z direction
	v.z -= 9.8f*dt;
	v2.z -= 9.8f*dt;
	//update the position with the new velocity
	//p.x += v.x*dt;
	//p.y += v.y*dt;
	p.z += v.z*dt;
	p2.z += v2.z*dt;
	//store the updated life in the velocity array
	v.w = life;
	v2.w = life2;


	//update the arrays with our newly computed values
	pos[i] = p;
	pos[i_secondary] = p2;
	vel[i] = v;
	vel[i_secondary] = v2;

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
		color[i].w = life;
	}
	color[i_secondary].w = life2;
}