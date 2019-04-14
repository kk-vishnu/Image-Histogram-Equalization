__kernel void calc_probability(__global int *intensity_frequency, __global float *intensity_probability, int dimensions){
	
	int i=get_global_id(0);	
		
	intensity_probability[i] = ((float)intensity_frequency[i]) / dimensions ;
			
}