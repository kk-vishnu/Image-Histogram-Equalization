#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
     
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "stb_image.h"
#define max_intensity 256
#define CHANNEL_NUM 1

int main(){
	clock_t start,end;
	double time;
	int width, height, bpp;
    char filename[30];
    printf("Enter image name:\n");
    scanf("%s",filename);
    uint8_t* rgb_image = stbi_load(filename, &width, &height, &bpp, 1);
    printf("Height:%d\tWidth:%d\tNumber of pixels:%d\n",height,width,height*width);
    

    int i=0,min=rgb_image[0],max=rgb_image[0],num_of_pixels=width*height;
    int intensity_limit;
    
    for(int i=0;i<num_of_pixels;i++){
    	if(rgb_image[i]<min)
    		min=rgb_image[i];
    	if(rgb_image[i]>max)
    		max=rgb_image[i];
    	
    }
    
	printf("Intensity range:%d-%d\n",min,max);
    printf("Enter new upper intensity limit:");
    scanf("%d",&intensity_limit);
    
    //printf("\nlimit:%d\n",intensity_limit);
    
	int *pixels=(int *)malloc(sizeof(int)*num_of_pixels);
	int *newpixels=(int *)malloc(sizeof(int)*num_of_pixels);
	int *frequencies=(int *)malloc(sizeof(int)*max_intensity);
	float *probability=(float *)malloc(sizeof(float)*max_intensity);
	float *cumulative=(float *)malloc(sizeof(float)*max_intensity);
	int *roundoff=(int *)malloc(sizeof(int)*max_intensity);

	start=clock();
	
	for(int i=0;i<max_intensity;i++){
		frequencies[i]=0;
		//printf("%d %d\n",i,pixels[i]);
	}

	for(int i=0;i<num_of_pixels;i++){
		pixels[i]=rgb_image[i];
	}
	stbi_image_free(rgb_image);


	for(int i=0;i<num_of_pixels;i++){
		frequencies[pixels[i]]++;
		//printf("%d %d\n",i,pixels[i]);
	}
	for(int i=0;i<max_intensity;i++){
		//printf("%d:%d\n",i,frequencies[i]);
		probability[i]=(1.0*frequencies[i])/num_of_pixels;
		//printf("%d:%f\n",i,probability[i]);
	}
	cumulative[0]=probability[0];
	for(int i=1;i<max_intensity;i++){
		cumulative[i]=cumulative[i-1]+probability[i];
		//printf("%d:%f\n",i,cumulative[i]);
	}
	for(int i=0;i<max_intensity;i++){
		cumulative[i]*=intensity_limit;
		//printf("%d:%\n",i,roundoff[i]);
		roundoff[i]=(int)cumulative[i];
		

	}	
	for(int i=0;i<num_of_pixels;i++){
		newpixels[i]=roundoff[pixels[i]];
	}	

	end=clock();

	time=((double)(end-start))/CLOCKS_PER_SEC;
	uint8_t* newimage=malloc(width*height*CHANNEL_NUM);
	
	i=0;
	min=max=newpixels[0];
    do{	
    	int x=newpixels[i];
    	if(x>max)
    		max=x;
    	if(x<min)
    		min=x;	
    	newimage[i]=x;
    	i++;	
    }while(i<num_of_pixels);
    //printf("%d\n",num_of_pixels);
    printf("Time:%f milliseconds",time*1000);
    printf("\nNew intensity range:%d-%d\n",min,max);	
    int ret=stbi_write_png("finalimage.png", width, height, CHANNEL_NUM, newimage, width*CHANNEL_NUM);
	
	return 0;
}