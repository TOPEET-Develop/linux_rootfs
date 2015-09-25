#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include "camera.h"

#define DEV_NAME_LENGTH		50
#define NUM_FRAM		50

int main(int argc, char ** argv) {
    //char *dev_name = "/dev/video0";	//ITU
    //char *dev_name = "/dev/video4";	//UVC
    char *dev_name = NULL;
    FILE * outf = 0;
    unsigned int image_size;

    int camera_type = 0;//0:ITU, 1:UVC	
    int width=640;
    int height=480;

    if(argc != 3)
    {
	printf("usage: ./camera /dev/video0 640x480\n");
	
	return 0;
    }

	dev_name = (char *)malloc(sizeof(char) * DEV_NAME_LENGTH);
	if(!dev_name)
	{
		printf("malloc mem error\n");
		return -1;
	}

    	memset(dev_name, 0, sizeof(char) * DEV_NAME_LENGTH);
	strcpy(dev_name, argv[1]);

	if(!strcmp(dev_name, "/dev/video4"))
	{
		camera_type = 1;
	}
	else
	{
		camera_type = 0;
	}

	if(!strcmp(argv[2], "640x480"))
	{
		width = 640;
    		height = 480;
	}
	else if(!strcmp(argv[2], "800x600"))
	{
		width = 800;
                height = 600;
	}
	else
	{
		width = 640;
                height = 480;
	}

    outf = fopen("out.yuv", "wb");
    Camera *camera;
    unsigned char image[width*height*2];

    clock_t starttime, endtime;
    double totaltime;

    camera=new Camera(dev_name,width,height, camera_type);
    if(!camera->OpenDevice()){
	printf("fun:%s, line = %d\n", __FUNCTION__, __LINE__);
        return -1;
    }
	printf("fun:%s, line = %d\n", __FUNCTION__, __LINE__);
    image_size=camera->getImageSize();
	printf("fun:%s, line = %d\n", __FUNCTION__, __LINE__);
    starttime = clock();
    //int frames=50;
    unsigned int writesize=0;
	printf("fun:%s, line = %d\n", __FUNCTION__, __LINE__);
    for(int i=0;i<NUM_FRAM;i++){
        if(!camera->GetBuffer(image)){
		printf("fun:%s, line = %d\n", __FUNCTION__, __LINE__);
            break;
        }
		printf("fun:%s, line = %d\n", __FUNCTION__, __LINE__);
        writesize=fwrite(image,1,image_size,outf);
        //fflush(outf);
        printf("frame:%d,writesize:%d\n",i,writesize);
    }

	printf("fun:%s, line = %d\n", __FUNCTION__, __LINE__);
    endtime = clock();
    totaltime = (double)( (endtime - starttime)/(double)CLOCKS_PER_SEC );
    printf("time :%f, rate :%f\n",totaltime,NUM_FRAM/totaltime);
    camera->CloseDevice();
    fclose(outf);
    return 0;
}

