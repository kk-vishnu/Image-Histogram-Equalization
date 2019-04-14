__kernel void calc_new_intensity(__global float *intensity_cumulative_probability, __global int *replacement_intensity, int new_max_intensity){
	
	int i=get_global_id(0);	
	
	replacement_intensity[i] = intensity_cumulative_probability[i] * new_max_intensity ;
			
}