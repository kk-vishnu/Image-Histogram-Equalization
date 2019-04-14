__kernel void calc_cumulative_probability(__global float *intensity_probability, __global float *intensity_cumulative_probability){
	
	int i=get_global_id(0);	
	
	float res=0;
	
	for(int j=0; j<=i; j++)
		res+=intensity_probability[j];
		
	intensity_cumulative_probability[i] = res ;
			
}
