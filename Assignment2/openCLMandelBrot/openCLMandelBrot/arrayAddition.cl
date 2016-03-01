__kernel void addArray(__global float *A, __global float *B, __global float *C)
{ 
	int width = 10;
	int height = 100;
	int i = get_global_id(0);
	int j = get_global_id(1);
	C[i*width+j] = A[i*width+j] + B[i*width+j];
}