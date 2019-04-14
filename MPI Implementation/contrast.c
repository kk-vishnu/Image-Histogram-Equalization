#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc,char *argv[]){
	clock_t start,end;	
	int rank,size;
	FILE *fptr;
	int max_intensity=256,num_of_pixels=0;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	
	if(rank==0){
		fptr=fopen("config","r");
		if(!fptr){
			printf("Error: config file not found");
			exit(0);
		}
		getw(fptr);
		int height=getw(fptr);
		int width=getw(fptr);
		num_of_pixels=height*width;
		fclose(fptr);
	}	

	MPI_Bcast(&num_of_pixels,1,MPI_INT,0,MPI_COMM_WORLD);
	int ratio=num_of_pixels/size;

	int *intensities=(int *)malloc(sizeof(int)*max_intensity);
	int *pixels=(int *)malloc(sizeof(int)*num_of_pixels);
	int *newpixels=(int *)malloc(sizeof(int)*num_of_pixels);
	int *oldpixels=(int *)malloc(sizeof(int)*ratio);
	int *convertedpixels=(int *)malloc(sizeof(int)*ratio);
	
	//Read original pixels,new intensities from file
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
		}while(x!=EOF);
		fclose(fptr);
		fptr=fopen("file2","r");
		if(!fptr){
			printf("Error: input file not found");
			exit(0);
		}
		x=getw(fptr);
		i=0;
		do{
			intensities[i++]=x;
			x=getw(fptr);
		}while(x!=EOF);
		fclose(fptr);
	}
	
	start=clock();
	
	MPI_Scatter(pixels,ratio,MPI_INT,oldpixels,ratio,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(intensities,max_intensity,MPI_INT,0,MPI_COMM_WORLD);
	
	//Convert pixels
	for(int i=0;i<ratio;i++)
		convertedpixels[i]=intensities[oldpixels[i]];
	
	MPI_Gather(convertedpixels,ratio,MPI_INT,newpixels,ratio,MPI_INT,0,MPI_COMM_WORLD);
	
	end=clock();
	
	//Write final pixels to file
	if(rank==0){
		float time=((float)(end-start))/CLOCKS_PER_SEC*1000;
		printf("\nTime-log 3:\t%0.03f milliseconds\n",time);
		fptr=fopen("finalpixels","w");
		for(int i=0;i<num_of_pixels;i++){
			putw(newpixels[i],fptr);
		}
		fclose(fptr);

		fptr=fopen("time_calc","r");
		float temp=0;
		fscanf(fptr, "%f", &temp);
		time+=temp;
		printf("\nTotal-Time:\t%0.03f milliseconds\n",time);
		fclose(fptr);

	}

	MPI_Finalize();
}