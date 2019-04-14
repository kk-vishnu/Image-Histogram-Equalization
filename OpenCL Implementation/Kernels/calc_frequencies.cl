__kernel void calc_frequency(__global int *pixel_matrix, __global int *intensity_frequency){
	
	int i=get_global_id(0);
		
	int pixel_value=pixel_matrix[i];
		
	atomic_add(&intensity_frequency[pixel_value] ,1);
		
}