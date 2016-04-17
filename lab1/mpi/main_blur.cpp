#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "ppmio.h"
#include "gaussw.h"

#define MAX_MEM 1024*1024
#define MAX_RAD 1000
#define ROOT_ID 1

/* NOTE: This structure must not be padded! */
typedef struct _pixel {
    unsigned char r,g,b;
} pixel; 

using namespace std;
	
void blur_chunk( pixel *buf, int buf_size, int myid, int interval) {

	int radius = 5;
    double w[MAX_RAD];
	
    get_gauss_weights(radius, w);

	printf( "Myid is %d, r = %d, g = %d, b = %d\n", myid, buf[0].r, buf[0].g, buf[0].b ); 
}

void blur_main(int myid, int p_tot, char *img_path) {
	MPI_Comm comm = MPI_COMM_WORLD;
	// Load image to buffer
	
	int x_size, y_size, max;
	int *scounts, *displs;
	pixel *data;

	if ( myid == ROOT_ID ) {

        data = (pixel *)malloc(MAX_PIXELS*sizeof(pixel));

		read_ppm( img_path, &x_size, &y_size, &max, (char *) data);

		int size = x_size * y_size;
		int chunk_size = size / p_tot;
		int last_chunk_pad = size - chunk_size * p_tot;

        displs = (int *)malloc(p_tot*sizeof(int));
        scounts = (int *)malloc(p_tot*sizeof(int));

		for( int i = 0; i < p_tot - 1; i++){
			displs[i] = i*chunk_size*3;
			scounts[i] = chunk_size;
		}

		scounts[p_tot-1] = chunk_size + last_chunk_pad; 

	}


    MPI_Bcast( &x_size, 1, MPI_INT, ROOT_ID, comm );

    int recv_count;

    MPI_Scatter(scounts, 1, MPI_INT, &recv_count, 1, MPI_INT, ROOT_ID, comm); 

	pixel recv_buf[ recv_count ];

	MPI_Scatterv( data, scounts, displs, MPI_CHAR, recv_buf, recv_count*3, MPI_CHAR,
			ROOT_ID, comm);
	
    blur_chunk( recv_buf, recv_count, myid, x_size );

	//
	//MPI_Send( "HELLO", 5, MPI_CHAR, 3, 1234, MPI_COMM_WORLD );
	
    if( myid == ROOT_ID ){
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

	blur_main( myid, p_tot, argv[1] );

	MPI_Finalize();
	return 0;
}

