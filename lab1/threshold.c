#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "ppm.h"

/* Pixel Declaration */
#define PIXEL(image,x,y) ((image)+((y)*(xsize)+(x)))

/* NOTE: This structure must not be padded! */
typedef struct _pixel {
    unsigned char r,g,b;
} pixel;

typedef struct _th_data {
	int start_idx;
	int count;
	int colmax;
	int image_size;
	unsigned tot_threads;
	pixel *image;

} th_data;

pthread_mutex_t avg_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barr;

unsigned avg = 0;
unsigned avg_threads_done = 0;

pixel *allocate_image(int size)
{
    pixel *image;
    image = (pixel *)malloc(sizeof(pixel)*size);
    if (!image) {
	fprintf(stderr, "malloc failed");
	exit(1);
    }
    return image;
}

void *filter(void *arg) {

    unsigned pval, sum = 0;

	th_data *data = (th_data *)arg;

	pixel *image = data->image;
	int start_idx = data->start_idx;
	int count = data->count;
    int colmax = data->colmax;
	int image_size = data->image_size;
	unsigned tot_threads = data->tot_threads;

    /* filter */

    for (int i = start_idx; i < start_idx + count; i++) {
	    sum += image[i].r;
	    sum += image[i].g;
	    sum += image[i].b;
    }

	pthread_mutex_lock( &avg_mutex );
    
	avg += sum;
	avg_threads_done++;

	if(avg_threads_done >= tot_threads){
		avg = avg/image_size;
	}
	
	pthread_mutex_unlock( &avg_mutex );

    // Synchronization point
    int rc = pthread_barrier_wait(&barr);
    if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        printf("Could not wait on barrier\n");
        exit(-1);
    }
	
	for (int i = start_idx; i < start_idx + count; i++) {
		if (sum > avg)
			pval = colmax;
		else
			pval = 0;
		image[i].r = pval;
		image[i].g = pval;
		image[i].b = pval;
	}
}


int main(int argc, char **argv)
{
    FILE *infile, *outfile;
    int magic, ok;
    int radius;
    int xsize, ysize, colmax;
    pixel *image;

	int threads = 1;

    /* Take care of the arguments */

    if (argc > 2) {
	fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
	exit(2);
    }
    if (!(infile = fopen(argv[1], "r"))) {
	fprintf(stderr, "Error when opening %s\n", argv[1]);
	exit(1);
    } 
    if (!(outfile = fopen(argv[2], "w"))) {
	fprintf(stderr, "Error when opening %s\n", argv[2]);
	exit(1);
    }
	if (argc == 4) {
    	threads = atoi(argv[3]);
		if(threads == 0){
        	fprintf(stderr, "Not a valid number of threads\n");
		}
	}


    /* read file */

    magic = ppm_readmagicnumber(infile);
    if (magic != 'P'*256+'6') {
	fprintf(stderr, "Wrong magic number\n");
	exit(1);
    }
    xsize = ppm_readint(infile);
    ysize = ppm_readint(infile);
    colmax = ppm_readint(infile);
    if (colmax > 255) {
	fprintf(stderr, "Too large maximum color-component value\n");
	exit(1);
    }
    
    image = allocate_image(xsize*ysize);
    if (!fread(image, sizeof(pixel), xsize*ysize, infile)) {
	fprintf(stderr, "error in fread\n");
	exit(1);
    }

    pthread_t thread_pool[threads];

	int rc;

	int size = xsize * ysize;
	int chunk_size = size / threads;
	int last_chunk_pad = size - chunk_size * threads;

	int displs[threads];

	int scounts[threads];

	for( int i = 0; i < threads; i++){
		displs[i] = i*chunk_size*3;
		scounts[i] = chunk_size*3;
	}

	scounts[threads-1] = (chunk_size + last_chunk_pad)*3;

	// Barrier initialization
    if(pthread_barrier_init(&barr, NULL, threads))
    {
        printf("Could not create a barrier\n");
        exit(-1);
    }

	for(int t = 0; t < threads; t++) {
		printf("Main: creating thread %d\n", t);

        th_data data;

		data.start_idx = displs[t];
		data.count = scounts[t];

		data.colmax = colmax;
		data.image_size = size;
		data.tot_threads = threads;

		data.image = image;

		rc = pthread_create(&thread_pool[t], NULL, filter, (void *)&data); 
		if (rc) {
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	for(int t = 0; t < threads; t++) {
    	pthread_join(thread_pool[t], NULL);
	}
    /* write result */
    
    fprintf(outfile, "P6 %d %d %d\n", xsize, ysize, colmax);
    if (!fwrite(image, sizeof(pixel), xsize*ysize, outfile)) {
	fprintf(stderr, "error in fwrite");
	exit(1);
    }

    exit(0);
}
