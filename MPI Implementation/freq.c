#include "mpi.h"
#include<stdio.h>
#include<stdlib.h>
#include <time.h>
#define max_intensity 256
int main(int argc,char *argv[]){
	clock_t start,end;
	int rank,size;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	FILE *fptr;

	int num_of_pixels=0;
	
	//Read image size from file
	
	if(rank==0){
		fptr=fopen("config","r");
		if(!fptr){
			printf("Error: config file not found\n");
			exit(0);
		}
		getw(fptr);
		int height=getw(fptr);
		int width=getw(fptr);
		num_of_pixels=width*height;
		fclose(fptr);
	}		
	
	MPI_Bcast(&num_of_pixels,1,MPI_INT,0,MPI_COMM_WORLD);	
	
	int ratio=num_of_pixels/size;
	int *pixelval=(int *)malloc(sizeof(int)*ratio);
	int *pixels=(int *)malloc(sizeof(int)*num_of_pixels);
	int *pixelfrequency=(int *)malloc(sizeof(int)*max_intensity);
	int *finfrequency=(int *)malloc(sizeof(int)*max_intensity);

	for(int i=0;i<max_intensity;i++)
		pixelfrequency[i]=0;
	
	//Read the pixels of image from file
	if(rank==0){
		fptr=fopen("file1","r");
		if(!fptr){
			printf("Error: input file not found");
			exit(0);
		}
		int x=getw(fptr);
		int i=0;
		do{	
			pixels[i++]=x;
			x=getw(fptr);
		}while(i<num_of_pixels);
		fclose(fptr);
		if(num_of_pixels%size!=0){
			printf("Processes mismatch\n");
			exit(0);
		}
	}	

	start=clock();
		
	MPI_Scatter(pixels,ratio,MPI_INT,pixelval,ratio,MPI_INT,0,MPI_COMM_WORLD);
	
	//Find frequency
	for(int i=0;i<ratio;i++){
		pixelfrequency[pixelval[i]]+=1;
	}

	MPI_Reduce(pixelfrequency,finfrequency,max_intensity,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
	
	end=clock();
	
	//Write the frequencies to file
	if(rank==0){
		float time=((float)(end-start))/CLOCKS_PER_SEC*1000;
		printf("\nTime-log 1:\t%0.03f milliseconds\n",time);
		fptr=fopen("op1","w");
		for(int i=0;i<max_intensity;i++){
			putw(finfrequency[i],fptr);
		}
		fclose(fptr);	

		fptr=fopen("time_calc","w");
		fprintf(fptr, "%0.03f\n", time);
		fclose(fptr);
	}

	MPI_Finalize();

	return 0;
}