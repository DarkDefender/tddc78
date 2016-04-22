#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "ppmio.h"

#define ROOT_ID 0

/* NOTE: This structure must not be padded! */
typedef struct _pixel {
	unsigned char r,g,b;
} pixel;

using namespace std;

void check_alloc(void *ptr){
	if(ptr == NULL){
		fprintf(stderr, "out of memory\n");
	}
}

unsigned sum_chunk( pixel *buf, int char_buf_size) {
	unsigned sum = 0;
	int buf_size = char_buf_size/3;

	for( int i = 0; i < buf_size; i++){
		sum += (unsigned)(buf[i].r + buf[i].g + buf[i].b);
	}

	sum = sum / buf_size;

	return sum;
}

void thres_chunk( pixel *buf, int char_buf_size, unsigned tot_sum) {
	unsigned sum;
	int buf_size = char_buf_size/3;

	for( int i = 0; i < buf_size; i++){
		sum = (unsigned)(buf[i].r + buf[i].g + buf[i].b);

		if( tot_sum > sum ){
			buf[i].r = buf[i].g = buf[i].b = 0;
		} else {
			buf[i].r = buf[i].g = buf[i].b = 255;
		}
	}

}

void thres_main(int myid, int p_tot, char *img_path) {
	MPI_Comm comm = MPI_COMM_WORLD;
	// Load image to buffer

	int x_size, y_size, max;
	int *scounts, *displs;
	pixel *data;

	if ( myid == ROOT_ID ) {

		data = (pixel *)malloc(MAX_PIXELS*sizeof(pixel));

		check_alloc(data);

		read_ppm( img_path, &x_size, &y_size, &max, (char *) data);

		int size = x_size * y_size;
		int chunk_size = size / p_tot;
		int last_chunk_pad = size - chunk_size * p_tot;

		displs = (int *)malloc(p_tot*sizeof(int));
		check_alloc(displs);

		scounts = (int *)malloc(p_tot*sizeof(int));

		check_alloc(scounts);

		for( int i = 0; i < p_tot; i++){
			displs[i] = i*chunk_size*3;
			scounts[i] = chunk_size*3;
		}

		scounts[p_tot-1] = (chunk_size + last_chunk_pad)*3;

	}

	int recv_count;

	MPI_Scatter(scounts, 1, MPI_INT, &recv_count, 1, MPI_INT, ROOT_ID, comm);

	pixel *recv_buf;

	recv_buf = (pixel *)malloc(recv_count*sizeof(char));
	check_alloc(recv_buf);

	MPI_Scatterv( data, scounts, displs, MPI_CHAR, recv_buf, recv_count, MPI_CHAR,
			ROOT_ID, comm);

	unsigned sum, tot_sum;

	sum = sum_chunk( recv_buf, recv_count );

	//printf("Sum for id:%d, sum:%d\n", myid, sum);

	MPI_Allreduce(&sum, &tot_sum, 1, MPI_UNSIGNED, MPI_SUM, comm);

	tot_sum = tot_sum / p_tot;

	thres_chunk( recv_buf, recv_count, tot_sum );

	MPI_Gatherv( recv_buf, recv_count, MPI_CHAR, data, scounts, displs, MPI_CHAR, ROOT_ID, comm);

	free(recv_buf);

	if( myid == ROOT_ID ){
		//printf("Totsum: %d\n", tot_sum);
		write_ppm( "./out.ppm", x_size, y_size, (char *) data);

		free(data);
		free(displs);
		free(scounts);
	}

}

int main( int argc, char **argv )
{
	struct stat buffer;
	if( argc != 2 || stat (argv[1], &buffer) != 0 ){
		printf("No image path/file specified or file doesn't exists\n");
		return -1;
	}

	int myid, p_tot;
	MPI_Init( NULL, NULL );
	MPI_Comm_rank( MPI_COMM_WORLD, &myid );
	MPI_Comm_size( MPI_COMM_WORLD, &p_tot );

	double starttime, endtime;
	starttime = MPI_Wtime();

	thres_main( myid, p_tot, argv[1] );

	endtime = MPI_Wtime();
	printf("That took %f seconds on id:%d\n", endtime-starttime, myid );

	MPI_Finalize();
	return 0;
}

