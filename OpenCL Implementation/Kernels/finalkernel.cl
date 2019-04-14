__kernel void calc_final_matrix(__global int *pixel_matrix, __global int *replacement_values, __global int *final_matrix){
	
	int i=get_global_id(0);	
	
	int val=pixel_matrix[i];
	
	final_matrix[i]=replacement_values[val];
				
}