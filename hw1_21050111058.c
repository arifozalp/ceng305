/**
 *
 * CENG305 Homework-1
 *
 * Histogram equalization with pthreads
 *
 * Usage:  make run IMAGE=<image file> THREAD=<number of thread>	
 *
 * @author  Arif Özalp
 *
 * @version 1.1, 08 November 2024
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"
#define CHANNEL_NUM 1

typedef struct {
    int thread_id;
    int width;
    int height;
    int num_threads;
    uint8_t *rgb_image;
} thread_args;

pthread_mutex_t lock;

// Global Variables - Do not touch them
int hist[256];
int cumhistogram[256];
int alpha[256];

void seq_histogram_equalizer(uint8_t* rgb_image,int width, int height);
void par_histogram_equalizer(uint8_t* rgb_image,int width, int height,int num_threads);

int main(int argc,char* argv[]) 
{		
    int width, height, bpp;
    int num_threads = atol(argv[2]);

    clock_t start, end;
    double elapsed_time;
	
	// Reading the image in grey colors
    uint8_t* rgb_image_for_seq = stbi_load(argv[1], &width, &height, &bpp, CHANNEL_NUM);
    uint8_t* rgb_image_for_par = stbi_load(argv[1], &width, &height, &bpp, CHANNEL_NUM);
	
    printf("Width: %d  Height: %d \n",width,height);
	printf("Input: %s , threads: %d  \n",argv[1],num_threads);
	
    ////////////////////////////////////////////////////////////////
    //
    // Sequential Code Starts
    //
    ////////////////////////////////////////////////////////////////

	// start the timer
    start = clock();
		
	seq_histogram_equalizer(rgb_image_for_seq,width, height);
    
	end = clock();
    elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000; // Convert to milliseconds
	printf("Sequential elapsed time: %lf \n",elapsed_time);

    ////////////////////////////////////////////////////////////////
    //
    // Multithreaded Code Starts
    //
    ////////////////////////////////////////////////////////////////

	//Here I reset the seq_histogram_equalizer function because it changes the values ​​of the hist array
	for (int i = 0; i < 256; i++) {
        	hist[i] = 0;
    	}	

    start = clock();
		
	par_histogram_equalizer(rgb_image_for_par,width, height,num_threads);
    
	end = clock();
    elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000; // Convert to milliseconds
	printf("Multi-thread elapsed time with %d threads: %lf \n",num_threads,elapsed_time);	

    // Check correctness of outputs here
    // ??
	
	// Storing the image 
    stbi_write_jpg("sequential_output.jpg", width, height, CHANNEL_NUM, rgb_image_for_seq, 100);
    stbi_write_jpg("multithread_output.jpg", width, height, CHANNEL_NUM, rgb_image_for_par, 100);
    stbi_image_free(rgb_image_for_seq);
    stbi_image_free(rgb_image_for_par);

    return 0;
}


void* threadFunction(void* args) {
    thread_args* t_args = (thread_args*) args;

    int width = t_args->width;
    int height = t_args->height;
    int num_threads = t_args->num_threads;
    uint8_t* rgb_image = t_args->rgb_image;

    int rows_per_thread = height / num_threads;
    int start_row = t_args->thread_id * rows_per_thread;
    int end_row = start_row + rows_per_thread;
    
    pthread_mutex_lock(&lock);
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < width; j++) {
            hist[rgb_image[i * width + j]]++; 
        }
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}

void par_histogram_equalizer(uint8_t* rgb_image, int width, int height, int num_threads) {
    pthread_t threads[num_threads];
    thread_args args[num_threads];
	
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Mutex init failed\n");
        return;
    }
    
    for (int i = 0; i < num_threads; i++) {
        int error;
        args[i].thread_id = i;
        args[i].width = width;
        args[i].height = height;
        args[i].num_threads = num_threads;
        args[i].rgb_image = rgb_image;
        error = pthread_create(&(threads[i]), NULL, &threadFunction, &args[i]);
        if(error !=0)
            printf("\nThread can't be created: [%s]", strerror(error));
    }
    
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    double size = width * height;
    cumhistogram[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cumhistogram[i] = hist[i] + cumhistogram[i - 1];
    }
    
    for (int i = 0; i < 256; i++) {
        alpha[i] = round((double)cumhistogram[i] * (255.0 / size));
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            rgb_image[i * width + j] = alpha[rgb_image[i * width + j]];
        }
    }

    pthread_mutex_destroy(&lock);
}


void seq_histogram_equalizer(uint8_t* rgb_image,int width, int height)
{			
	for(int i=0; i<height ; i++){
		for(int j=0; j<width; j++){
			hist[rgb_image[i*width + j]]++;
		}
	}	
	double size = width * height;
   
     //cumulative sum for histogram values    
	cumhistogram[0] = hist[0];
    for(int i = 1; i < 256; i++)
    {
        cumhistogram[i] = hist[i] + cumhistogram[i-1];
    }    
	    
    for(int i = 0; i < 256; i++)
    {
        alpha[i] = round((double)cumhistogram[i] * (255.0/size));
    }
			
    // histogram equlized image		
    for(int y = 0; y <height ; y++){
        for(int x = 0; x < width; x++){
            rgb_image[y*width + x] = alpha[rgb_image[y*width + x]];
		}
	}
}

