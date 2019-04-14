#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define max_intensity 256

int main(int argc,char* argv[]){	
	clock_t start,end;
	int rank,size;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	
	if(max_intensity%size!=0){
		printf("Number of processes mismatched");
		exit(0);
	}

	int ratio=max_intensity/size;
	
	int total_pixels,intensity;
	float recv=0.0,temp;
	int *intermediate_sum=(int *)malloc(sizeof(int)*ratio);
	int *pixels=(int *)malloc(sizeof(int)*ratio);
	float *probability=(float *)malloc(sizeof(float)*ratio);
	float *cumulative=(float *)malloc(sizeof(float)*ratio);
	int *frequencies=(int *)malloc(max_intensity*(sizeof(int)));
	float *cum_probs=(float *)malloc(max_intensity*(sizeof(float)));
	int *rounding=(int *)malloc(max_intensity*(sizeof(int)));
	int *roundoff=(int *)malloc(sizeof(int)*ratio);
	FILE *fptr;

	//Read intensity frequencies from file
	if(rank==0)
	{	fptr=fopen("op1","r");
		if(!fptr){
			printf("File not found");
			exit(0);
		}
		for(int i=0;i<max_intensity;i++){
			frequencies[i]=getw(fptr);
		}
		fclose(fptr);
		fptr=fopen("config","r");
		if(!fptr){
			printf("Config file missing\n");
			exit(0);
		}
		intensity=getw(fptr);
		int height=getw(fptr);
		int width=getw(fptr);
		total_pixels=height*width;
		fclose(fptr);
	}
	
	start=clock();
	
	MPI_Bcast(&total_pixels,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&intensity,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Scatter(frequencies,ratio,MPI_INT,pixels,ratio,MPI_INT,0,MPI_COMM_WORLD);

	//Find probability
	for(int i=0;i<ratio;i++)
		probability[i]=pixels[i]*1.0/total_pixels;
	
	//Find cumulative probability within each process
	cumulative[0]=probability[0];
	for(int i=1;i<ratio;i++)
		cumulative[i]=probability[i]+cumulative[i-1];
	
	//Send in-process cumulative probability to next process
	if(rank<size-1){
		MPI_Send(&cumulative[ratio-1],1,MPI_FLOAT,rank+1,0,MPI_COMM_WORLD);
	}

	MPI_Status status;
	
	if(rank>0){
		MPI_Recv(&recv,1,MPI_FLOAT,rank-1,0,MPI_COMM_WORLD,&status);
	} 
	
	MPI_Scan(&recv,&temp,1,MPI_FLOAT,MPI_SUM,MPI_COMM_WORLD);
	
	//Find new pixel intensities
	for(int i=0;i<ratio;i++){
		cumulative[i]+=temp;
		cumulative[i]*=intensity;
		roundoff[i]=(int)cumulative[i];
	}

	MPI_Gather(roundoff,ratio,MPI_INT,rounding,ratio,MPI_INT,0,MPI_COMM_WORLD);
	
	end=clock();
	
	//Write final intensities to file
	if(rank==0){
		float time=((float)(end-start))/CLOCKS_PER_SEC*1000;
		printf("\nTime-log 2:\t%0.03f milliseconds\n",time);
		fptr=fopen("file2","w");
		for(int i=0;i<max_intensity;i++){
			putw(rounding[i],fptr);
		}
		fclose(fptr);

		float temp=0;
		fptr=fopen("time_calc","r");
		fscanf(fptr, "%f", &temp);
		fclose(fptr);		
		time+=temp;

		fptr=fopen("time_calc","w");
		fprintf(fptr, "%0.03f\n", time);
		fclose(fptr);
	}

	MPI_Finalize();
	return 0;
}
