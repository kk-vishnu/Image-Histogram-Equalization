#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#define CHANNEL_NUM 1

int main() {
    FILE *fptr;
    int width,height;
    
    fptr=fopen("config","r");
    if(!fptr){
    	printf("File info not found\n");
    	exit(0);
    }
    getw(fptr);
    height=getw(fptr);
    width=getw(fptr);

    uint8_t* rgb_image;
    rgb_image = malloc(width*height*CHANNEL_NUM);
    
    fptr=fopen("finalpixels","r");
    int i=0,x=getw(fptr);
    int min=x,max=x;
    do{	if(x>max)
    		max=x;
    	if(x<min)
    		min=x;	
    	rgb_image[i++]=x;
    	x=getw(fptr);
    }while(x!=EOF);
    printf("\nNew intensity range:%d-%d\n",min,max);
    int ret=stbi_write_png("finalimage.png", width, height, CHANNEL_NUM, rgb_image, width*CHANNEL_NUM);
    printf("\nImage saved as 'finalimage.png'\n");
    return 0;
}