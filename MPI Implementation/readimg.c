#include <stdint.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main() {
    int width, height, bpp;
    char filename[30];
    
    printf("Enter image name:\n");
    scanf("%s",filename);
    
    //Read the image
    uint8_t* rgb_image = stbi_load(filename, &width, &height, &bpp, 1);
    
    printf("Height:%d\tWidth:%d\tNumber of pixels:%d\n",height,width,height*width);
   
    int i=0,min=rgb_image[0],max=rgb_image[0],newintensity;
    
    //Write the pixels to a file
    FILE *fptr=fopen("file1","w");
    do{		
    	if(rgb_image[i]<min)
    		min=rgb_image[i];
    	if(rgb_image[i]>max)
    		max=rgb_image[i];
    	putw((int)rgb_image[i++],fptr);
    }while(i<width*height);
    fclose(fptr);
    
    printf("Intensity range:%d-%d\n",min,max);
    printf("\nEnter new upper intensity limit:");
    scanf("%d",&newintensity);
    stbi_image_free(rgb_image);
    
    //Write image details,new maximum intensity to a file
    fptr=fopen("config","w");
    putw(newintensity,fptr);
    putw(height,fptr);
    putw(width,fptr);
    fclose(fptr);
    return 0;
}
