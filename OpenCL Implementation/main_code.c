#include <stdio.h>
#include <CL/cl.h>
#include<stdlib.h>
#include <time.h>
#define MAX_SOURCE_SIZE (0x100000)

void eprint(cl_int ec, char *str) {
    if (ec != CL_SUCCESS) {
        printf("\nERROR: %d %s",ec ,str);
    }
}

int main(void) {
	//Initialize the required arrays
	cl_ulong time_start,time_end;
	int i,x;
	int dimensions,max_intensity;	
	int new_max_intensity;
	clock_t start,end;
	double inbtime,inbtot=0;

	FILE *fptr=fopen("config","r");
	if(!fptr){
		printf("Config file missing\n");
		exit(0);
	}
	new_max_intensity=getw(fptr);
	max_intensity=getw(fptr);
	int height=getw(fptr);
	int width=getw(fptr);
	max_intensity++;
	fclose(fptr);

	dimensions=height*width;

	int *pixel_matrix = (int*) malloc(sizeof(int) * dimensions);
	int *intensity_frequency = (int*) malloc(sizeof(int) * max_intensity);




	//Get pixel values
	/*printf("Enter the values: ");
	for (i = 0; i < dimensions; i++)
		scanf("%d", &pixel_matrix[i]);*/

	fptr=fopen("matrix","r");
	if(!fptr){
		printf("Error: input file not found");
		exit(0);
	}
	x=getw(fptr);
	i=0;
	do{	//printf("%d read\n",x);
		pixel_matrix[i++]=x;
		x=getw(fptr);
	}while(i<dimensions);


	//Set initial frequencies to zero
	for (i=0; i<max_intensity; i++)
		intensity_frequency[i]=0;



	// Calculating frequency now
	FILE *fp;
	char *source_str;
	size_t source_size;
	fp = fopen("Kernels/calc_frequencies.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		getchar();
		exit(1);
	}
	source_str = (char*) malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	// Get platform and device information
	cl_platform_id platform_id = NULL;
	cl_device_id device_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	eprint(ret,"clGetPlatformIDs");
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num_devices);
	eprint(ret,"clGetPlatformIDs");

	// Create an OpenCL context
	cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
	eprint(ret,"clCreatecontext");

	// Create a command queue
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret);
	eprint(ret,"clCreateCommandQueue1");

	// Create memory buffers on the device for pixel_matrix and intensity_frequency;
	cl_mem p_m_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, dimensions * sizeof(int), NULL, &ret);
	eprint(ret,"clCreateBuffer1");
	cl_mem i_f_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, max_intensity * sizeof(int), NULL, &ret);
	eprint(ret,"clCreateBuffer2");

	// Copy the arrays to their respective memory buffers
	ret = clEnqueueWriteBuffer(command_queue, p_m_mem_obj, CL_TRUE, 0, dimensions * sizeof(int), pixel_matrix, 0, NULL, NULL);
	eprint(ret,"clWriteBuffer1");
	ret = clEnqueueWriteBuffer(command_queue, i_f_mem_obj, CL_TRUE, 0, max_intensity * sizeof(int), intensity_frequency, 0, NULL, NULL);
	eprint(ret,"clWriteBuffer2");

	// Create a program from the kernel source
	cl_program program = clCreateProgramWithSource(context, 1, (const char**) &source_str, (const size_t *) &source_size, &ret);
	eprint(ret,"clCreateProgramWithSource");

	// Build the program
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	eprint(ret,"clBuildProgram");

	// Create the OpenCL kernel object
	cl_kernel kernel = clCreateKernel(program, "calc_frequency", &ret);
	eprint(ret,"clCreateKernel");

	// Set the arguments of the kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &p_m_mem_obj);
	eprint(ret,"clSetKernelArg1");
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *) &i_f_mem_obj);
	eprint(ret,"clSetKernelArg2");

	// Execute the OpenCL kernel on the array
	size_t global_item_size = dimensions;
	size_t local_item_size = 1;

	//Execute the kernel on the device
	cl_event event;
	start=clock();
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, &event);
	end=clock();
	inbtime=((double)(end-start))/CLOCKS_PER_SEC*1000;
	inbtot+=inbtime;
	eprint(ret,"clEnqueueNDRangeKernel");
	ret = clFinish(command_queue);
	printf("\nFrequencies time: \t\t\t%0.3f milliseconds\n",inbtime);

	// Read the memory buffer on the device to the local variable
	ret = clEnqueueReadBuffer(command_queue, i_f_mem_obj, CL_TRUE, 0, max_intensity * sizeof(int), intensity_frequency, 0, NULL, NULL);
	eprint(ret,"clEnqueueReadBuffer");

	// Display the result to the screen
	/*printf("\n\nFrequencies:\n");
	for (i = 0; i < max_intensity; i++)
		printf("%d -> %d\n", i, intensity_frequency[i]);
	*/





	//Calculating probabilities now
	fp = fopen("Kernels/calc_probabilities.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		getchar();
		exit(1);
	}
	source_str = (char*) malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	//Array to store probabilities
	float *intensity_probability = (float*) malloc(sizeof(float) * max_intensity);

	// Create memory buffers on the device for intensity_probability;
	cl_mem i_p_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, max_intensity * sizeof(float), NULL, &ret);
	cl_mem i_f2_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, max_intensity * sizeof(int), NULL, &ret);
	eprint(ret,"clCreateBuffer3");

	// Copy the array to it's memory buffers
	ret = clEnqueueWriteBuffer(command_queue, i_f2_mem_obj, CL_TRUE, 0, max_intensity * sizeof(float), intensity_frequency, 0, NULL, NULL);
	eprint(ret,"clWriteBuffer1");
	ret = clEnqueueWriteBuffer(command_queue, i_p_mem_obj, CL_TRUE, 0, max_intensity * sizeof(int), intensity_probability, 0, NULL, NULL);
	eprint(ret,"clWriteBuffer1");

	// Create a program from the kernel source
    program = clCreateProgramWithSource(context, 1, (const char**) &source_str, (const size_t *) &source_size, &ret);
	eprint(ret,"clCreateProgramWithSource");

	// Build the program
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	eprint(ret,"clBuildProgram");

	// Create the OpenCL kernel object
	kernel = clCreateKernel(program, "calc_probability", &ret);
	eprint(ret,"clCreateKernel");

	// Set the arguments of the kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &i_f2_mem_obj);
	eprint(ret,"clSetKernelArg2.1");
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *) &i_p_mem_obj);
	eprint(ret,"clSetKernelArg2.2");
	ret = clSetKernelArg(kernel, 2, sizeof(cl_int), (void *) &dimensions);
	eprint(ret,"clSetKernelArg2.3");

	// Execute the OpenCL kernel on the array
	global_item_size = max_intensity;
	local_item_size = 1;

	//Execute the kernel on the device
	start=clock();
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, &event);
	ret = clFinish(command_queue);	
	end=clock();
	inbtime=((double)(end-start))/CLOCKS_PER_SEC*1000;
	inbtot+=inbtime;
	eprint(ret,"clEnqueueNDRangeKernel");	
	printf("\nProbablities time: \t\t\t%0.3f milliseconds\n",inbtime);

	// Read the memory buffer on the device to the local variable
	ret = clEnqueueReadBuffer(command_queue, i_p_mem_obj, CL_TRUE, 0, max_intensity * sizeof(float), intensity_probability, 0, NULL, NULL);
	eprint(ret,"clEnqueueReadBuffer");

	// Display the result to the screen
	/*printf("\n\nProbablities:\n");
	for (i = 0; i < max_intensity; i++)
		printf("%d ->%d  %f\n", i,intensity_frequency[i] ,intensity_probability[i]);
	*/






	//Calculating cumulative probability
	fp = fopen("Kernels/calc_cumilative_probabilities.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		getchar();
		exit(1);
	}
	source_str = (char*) malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	//Array to store probabilities
	float *intensity_cumulative_probability = (float*) malloc(sizeof(float) * max_intensity);

	// Create memory buffers on the device for intensity_cumulative_probability;
	cl_mem i_c_p_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, max_intensity * sizeof(float), NULL, &ret);
	cl_mem i_p2_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, max_intensity * sizeof(float), NULL, &ret);

	// Copy the array to it's memory buffers
	ret = clEnqueueWriteBuffer(command_queue, i_p2_mem_obj, CL_TRUE, 0, max_intensity * sizeof(float), intensity_probability, 0, NULL, NULL);
	eprint(ret,"clWriteBuffer1");

	// Create a program from the kernel source
    program = clCreateProgramWithSource(context, 1, (const char**) &source_str, (const size_t *) &source_size, &ret);
	eprint(ret,"clCreateProgramWithSource");

	// Build the program
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	eprint(ret,"clBuildProgram");

	// Create the OpenCL kernel object
	kernel = clCreateKernel(program, "calc_cumulative_probability", &ret);
	eprint(ret,"clCreateKernel");

	// Set the arguments of the kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &i_p2_mem_obj);
	eprint(ret,"clSetKernelArg3.1");
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *) &i_c_p_mem_obj);
	eprint(ret,"clSetKernelArg3.2");

	// Execute the OpenCL kernel on the array
	global_item_size = max_intensity;
	local_item_size = 1;

	//Execute the kernel on the device
	start=clock();
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL,&event);
	end=clock();
	inbtime=((double)(end-start))/CLOCKS_PER_SEC*1000;
	inbtot+=inbtime;
	eprint(ret,"clEnqueueNDRangeKernel");
	ret = clFinish(command_queue);
	printf("\nCumulative Probabilities time: \t\t%0.3f milliseconds\n",inbtime);

	// Read the memory buffer on the device to the local variable
	ret = clEnqueueReadBuffer(command_queue, i_c_p_mem_obj, CL_TRUE, 0, max_intensity * sizeof(float), intensity_cumulative_probability, 0, NULL, NULL);
	eprint(ret,"clEnqueueReadBuffer");

	// Display the result to the screen
	/*printf("\n\nCumulative Probabilities:\n");
	for (i = 0; i < max_intensity; i++)
		printf("%d  %f  %f\n", i,intensity_probability[i] ,intensity_cumulative_probability[i]);
	*/






	//Calculating replacement values
	fp = fopen("Kernels/calc_new_intensities.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		getchar();
		exit(1);
	}
	source_str = (char*) malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	int *replacement_intensity = (int*) malloc(sizeof(int) * max_intensity);

	// Create memory buffers on the device for replacement_values
	cl_mem i_c_p2_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, max_intensity * sizeof(float), NULL, &ret);
	cl_mem r_i_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, max_intensity * sizeof(float), NULL, &ret);

	// Copy the array to it's memory buffers
	ret = clEnqueueWriteBuffer(command_queue, i_c_p2_mem_obj, CL_TRUE, 0, max_intensity * sizeof(float), intensity_cumulative_probability, 0, NULL, NULL);
	eprint(ret,"clWriteBuffer1");

	// Create a program from the kernel source
    program = clCreateProgramWithSource(context, 1, (const char**) &source_str, (const size_t *) &source_size, &ret);
	eprint(ret,"clCreateProgramWithSource");

	// Build the program
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	eprint(ret,"clBuildProgram");

	// Create the OpenCL kernel object
	kernel = clCreateKernel(program, "calc_new_intensity", &ret);
	eprint(ret,"clCreateKernel");

	// Set the arguments of the kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &i_c_p2_mem_obj);
	eprint(ret,"clSetKernelArg4.1");
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *) &r_i_mem_obj);
	eprint(ret,"clSetKernelArg4.2");
	ret = clSetKernelArg(kernel, 2, sizeof(cl_int), (void *) &new_max_intensity);
	eprint(ret,"clSetKernelArg4.2");

	// Execute the OpenCL kernel on the array
	global_item_size = max_intensity;
	local_item_size = 1;

	//Execute the kernel on the device
	start=clock();
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, &event);
	end=clock();
	inbtime=((double)(end-start))/CLOCKS_PER_SEC*1000;
	inbtot+=inbtime;
	eprint(ret,"clEnqueueNDRangeKernel");
	ret = clFinish(command_queue);
	printf("\nReplacement values time: \t\t%0.3f milliseconds\n",inbtime);

	// Read the memory buffer on the device to the local variable
	ret = clEnqueueReadBuffer(command_queue, r_i_mem_obj, CL_TRUE, 0, max_intensity * sizeof(int), replacement_intensity, 0, NULL, NULL);
	eprint(ret,"clEnqueueReadBuffer");

	// Display the result to the screen
	/*printf("\n\nReplacement Values:\n");
	for (i = 0; i < max_intensity; i++)
		printf("%d --> %d  \n", i,replacement_intensity[i]);
	*/










	//Final pixel values
	fp = fopen("Kernels/finalkernel.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		getchar();
		exit(1);
	}
	source_str = (char*) malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	int *final_matrix = (int*) malloc(sizeof(int) * dimensions);

	// Create memory buffers on the device for replacement_values
	cl_mem p_m2_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, dimensions * sizeof(int), NULL, &ret);
	cl_mem r_i2_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, max_intensity * sizeof(int), NULL, &ret);
	cl_mem f_m_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, dimensions * sizeof(int), NULL, &ret);

	// Copy the array to it's memory buffers
	ret = clEnqueueWriteBuffer(command_queue, r_i2_mem_obj, CL_TRUE, 0, max_intensity * sizeof(int), replacement_intensity, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, p_m2_mem_obj, CL_TRUE, 0, dimensions * sizeof(int), pixel_matrix, 0, NULL, NULL);
	eprint(ret,"clWriteBuffer1");

	// Create a program from the kernel source
    program = clCreateProgramWithSource(context, 1, (const char**) &source_str, (const size_t *) &source_size, &ret);
	eprint(ret,"clCreateProgramWithSource");

	// Build the program
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	eprint(ret,"clBuildProgram");

	// Create the OpenCL kernel object
	kernel = clCreateKernel(program, "calc_final_matrix", &ret);
	eprint(ret,"clCreateKernel");

	// Set the arguments of the kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &p_m2_mem_obj);
	eprint(ret,"clSetKernelArg5.1");
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *) &r_i2_mem_obj);
	eprint(ret,"clSetKernelArg5.1");
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *) &f_m_mem_obj);
	eprint(ret,"clSetKernelArg5.2");

	// Execute the OpenCL kernel on the array
	global_item_size = dimensions;
	local_item_size = 1;

	//Execute the kernel on the device
	start=clock();
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, &event);
	end=clock();
	inbtime=((double)(end-start))/CLOCKS_PER_SEC*1000;
	inbtot+=inbtime;
	eprint(ret,"clEnqueueNDRangeKernel");
	ret = clFinish(command_queue);
	printf("\nFinal Matrix time: \t\t\t%0.3f milliseconds\n",inbtime);

	// Read the memory buffer on the device to the local variable
	ret = clEnqueueReadBuffer(command_queue, f_m_mem_obj, CL_TRUE, 0, dimensions * sizeof(int), final_matrix, 0, NULL, NULL);
	eprint(ret,"clEnqueueReadBuffer");

	// Display the result to the screen
	/*printf("\nFinal Matrix:");
	for (i = 0; i < dimensions; i++){
		printf("%d ",final_matrix[i]);
	}*/

	fptr=fopen("finalpixels","w");
	for(i=0;i<dimensions;i++){
		putw(final_matrix[i],fptr);
		//printf("%d:%d \n",i,newpixels[i]);
	}
	fclose(fptr);

	printf("\nTotal time for execution: \t\t%0.3f milliseconds\n",inbtot);


	// Clean up
	ret = clFlush(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(p_m_mem_obj);
	ret = clReleaseMemObject(i_f_mem_obj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);
	return 0;
}


/*
3 2 4 5 7 7 8 2 3 1 2 3 5 4 6 7
*/
