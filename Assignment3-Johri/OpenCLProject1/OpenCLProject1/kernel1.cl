__kernel void reduction(__global long long *gdata, __local long long*sdata) {
   unsigned int tid = get_local_id(0);
   unsigned int gid = get_global_id(0);

   // copy int’s from Global to Local memory
   sdata[tid] = gdata[gid];

   // do reduction in Local memory
   for(unsigned int s=1; s < get_local_size(0); s *= 2) {
      if (tid % (2*s) == 0){ // if ThreadID is a multiple of 2*s
         sdata[tid] += sdata[tid + s];
      }
	  barrier(CLK_LOCAL_MEM_FENCE);
   }

   // write result for this block to global mem
   if (tid == 0) gdata[get_group_id(0)] = sdata[0];
}
