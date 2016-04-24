#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "ppm.h"
#include "gaussw.h"

#define MAX_RAD 1000
#define RADIUS 5

/* NOTE: This structure must not be padded! */
typedef struct _pixel {
    unsigned char r,g,b;
} pixel;

typedef struct _th_data {
	int start_idx;
	int count;
	int x_size;
	int y_size;

	pixel *image;
	pixel *work_buf;

	double *w;

} th_data;

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

	th_data *data = (th_data *)arg;

	pixel *image = data->image;
	pixel *work_buf = data->work_buf;
	int start_idx = data->start_idx;
	int count = data->count;
	int x_size = data->x_size;
	int y_size = data->y_size;

    double *w = data->w;

	/* filter */
	double r,g,b,n,wc;
	int x, x2, y, y2;

	for (int i = start_idx; i < start_idx + count; i++) {
		r = w[0] * image[i].r;
		g = w[0] * image[i].g;
		b = w[0] * image[i].b;
		n = w[0];

		x = (start_idx + i) % x_size;

		for (int wi = 1; wi <= RADIUS; wi++) {
			wc = w[wi];
			x2 = x - wi;
			if(x2 >= 0) {
				r += wc * image[i - wi].r;
				g += wc * image[i - wi].g;
				b += wc * image[i - wi].b;
				n += wc;
			}
			x2 = x + wi;
			if(x2 < x_size) {
				r += wc * image[i + wi].r;
				g += wc * image[i + wi].g;
				b += wc * image[i + wi].b;
				n += wc;
			}
		}
		work_buf[i].r = r/n;
		work_buf[i].g = g/n;
		work_buf[i].b = b/n;
	}

	// Synchronization point
	int rc = pthread_barrier_wait(&barr);
	if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
	{
		printf("Could not wait on barrier\n");
		exit(-1);
	}

	for (int i = start_idx; i < start_idx + count; i++) {
		r = w[0] * work_buf[i].r;
		g = w[0] * work_buf[i].g;
		b = w[0] * work_buf[i].b;
		n = w[0];

		y = (start_idx + i) / x_size;

		for (int wi = 1; wi <= RADIUS; wi++) {
			wc = w[wi];
			y2 = y - wi;
			if(y2 >= 0) {
				r += wc * work_buf[i - wi * x_size].r;
				g += wc * work_buf[i - wi * x_size].g;
				b += wc * work_buf[i - wi * x_size].b;
				n += wc;
			}
			y2 = y + wi;
			if(y2 < y_size) {
				r += wc * work_buf[i + wi * x_size].r;
				g += wc * work_buf[i + wi * x_size].g;
				b += wc * work_buf[i + wi * x_size].b;
				n += wc;
			}
		}
		image[i].r = r/n;
		image[i].g = g/n;
		image[i].b = b/n;
	}
}


int main(int argc, char **argv)
{
    FILE *infile, *outfile;
    static int magic;
    static int xsize, ysize, colmax;
    pixel *image, *work_buf;

	int threads = 1;

    /* Take care of the arguments */

    if (argc < 3) {
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
			exit(1);
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

    //Allocate the work buffer needed for the blur algorithm
    work_buf = allocate_image(xsize*ysize);

	// Calculate the gauss array for blurring
	double w[MAX_RAD];
	get_gauss_weights(RADIUS, w);

    struct timespec stime, etime;
    clock_gettime(CLOCK_REALTIME, &stime);

    pthread_t thread_pool[threads];

	int rc;

	int size = xsize * ysize;
	int chunk_size = size / threads;
	int last_chunk_pad = size - chunk_size * threads;

	int displs[threads];

	int scounts[threads];

	for( int i = 0; i < threads; i++){
		displs[i] = i*chunk_size;
		scounts[i] = chunk_size;
	}

	scounts[threads-1] = (chunk_size + last_chunk_pad);

	// Barrier initialization
    if(pthread_barrier_init(&barr, NULL, threads))
    {
        printf("Could not create a barrier\n");
        exit(-1);
    }

	th_data data[threads];

	for(int t = 0; t < threads; t++) {
		//printf("Main: creating thread %d\n", t);

		data[t].start_idx = displs[t];
		data[t].count = scounts[t];

		data[t].x_size = xsize;
		data[t].y_size = ysize;

		data[t].image = image;
		data[t].work_buf = work_buf;
		data[t].w = w;

		rc = pthread_create(&thread_pool[t], NULL, filter, (void *)&data[t]); 
		if (rc) {
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	for(int t = 0; t < threads; t++) {
    	pthread_join(thread_pool[t], NULL);
	}

    clock_gettime(CLOCK_REALTIME, &etime);

    printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
	   1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

    /* write result */
    
    fprintf(outfile, "P6 %d %d %d\n", xsize, ysize, colmax);
    if (!fwrite(image, sizeof(pixel), xsize*ysize, outfile)) {
	fprintf(stderr, "error in fwrite");
	exit(1);
    }

    free(image);
	free(work_buf);

    exit(0);
}
