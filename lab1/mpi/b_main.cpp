#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "ppmio.h"
#include "gaussw.h"

#define MAX_MEM 1024*1024
#define MAX_RAD 1000
#define ROOT_ID 0

/* NOTE: This structure must not be padded! */
typedef struct _pixel {
	unsigned char r,g,b;
} pixel;

using namespace std;

void check_alloc(void *ptr, string mes){
	if(ptr == NULL){
		fprintf(stderr, "out of memory: %s\n", mes.c_str());
	}
}

void blur_chunk( pixel *buf, int char_buf_size, int myid, int p_tot, int x_size) {
	int buf_size = char_buf_size/3;

	int radius = 5;
	double w[MAX_RAD];
	double r,g,b,n,wc;

	pixel *tmp_buf;

	tmp_buf = (pixel *)malloc(buf_size*sizeof(pixel));
	check_alloc(tmp_buf, "tmp_buf");

	get_gauss_weights(radius, w);

    int start_x;
	
	if(myid != p_tot - 1){
		start_x = (myid * buf_size) % x_size;
	} else {
		start_x = buf_size % x_size;
		if( start_x != 0 ){
			start_x = x_size - start_x;  
		}
	}

	pixel *l_buf = NULL;
	pixel *r_buf = NULL;

	//0-1 = send req, 2-3 = recv req
	MPI_Status status[4];
	MPI_Request req[4];

	if( start_x != 0 ){

		l_buf = (pixel *)malloc(radius*sizeof(pixel));
		check_alloc(l_buf, "l_buf");

		//Ask left id for pixels
		MPI_Isend( buf, radius*3, MPI_CHAR, myid - 1, 1234, MPI_COMM_WORLD, &req[0] );
		MPI_Irecv( l_buf, radius*3, MPI_CHAR, myid - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &req[2] );

        printf("%d: l_buf to id %d\n", myid, myid - 1);

	}
	if( (start_x + buf_size) % x_size != 0 ){

		r_buf = (pixel *)malloc(radius*sizeof(pixel));
		check_alloc(r_buf, "r_buf");

		//Ask right id for pixels
		MPI_Isend( &buf[buf_size - radius - 1], radius*3, MPI_CHAR, myid + 1, 1234, MPI_COMM_WORLD, &req[1] );
		MPI_Irecv( r_buf, radius*3, MPI_CHAR, myid + 1, MPI_ANY_TAG, MPI_COMM_WORLD, &req[3] );

        printf("%d: r_buf to id %d\n", myid, myid + 1);
	}

    int x, x2;

	for(int i = radius; i < buf_size - radius; i++){
		r = w[0] * buf[i].r;
		g = w[0] * buf[i].g;
		b = w[0] * buf[i].b;
		n = w[0];

		x = (start_x + i) % x_size;

		for( int wi = 1; wi <= radius; wi++){
        	wc = w[wi];
			x2 = x - wi;
			if(x2 >= 0){
				r += wc * buf[i - wi].r;
				g += wc * buf[i - wi].g;
				b += wc * buf[i - wi].b;
				n += wc;
			}
			x2 = x + wi;
			if(x2 < x_size){
				r += wc * buf[i + wi].r;
				g += wc * buf[i + wi].g;
				b += wc * buf[i + wi].b;
				n += wc;
			}
		}
		tmp_buf[i].r = r/n;
		tmp_buf[i].g = g/n;
		tmp_buf[i].b = b/n;
	}



	if( l_buf != NULL ){
		MPI_Wait( &req[2], &status[2] );
	}

	for(int i = 0; i < radius; i++){
		r = w[0] * buf[i].r;
		g = w[0] * buf[i].g;
		b = w[0] * buf[i].b;
		n = w[0];

		x = (start_x + i) % x_size;

		for( int wi = 1; wi <= radius; wi++){
			wc = w[wi];
			x2 = x - wi;
			if(x2 >= 0){
				int idx = i - wi;
				if( idx >= 0 ){
					r += wc * buf[idx].r;
					g += wc * buf[idx].g;
					b += wc * buf[idx].b;
					n += wc;
				} else if( l_buf != NULL ) {
					r += wc * l_buf[radius + idx].r;
					g += wc * l_buf[radius + idx].g;
					b += wc * l_buf[radius + idx].b;
					n += wc;
				}
			}
			x2 = x + wi;
			if(x2 < x_size){
				r += wc * buf[i + wi].r;
				g += wc * buf[i + wi].g;
				b += wc * buf[i + wi].b;
				n += wc;
			}
		}
		tmp_buf[i].r = r/n;
		tmp_buf[i].g = g/n;
		tmp_buf[i].b = b/n;

	}
    
    if( r_buf != NULL ){
		MPI_Wait( &req[3], &status[3] );
	}

	for(int i = buf_size - radius; i < buf_size; i++){
		r = w[0] * buf[i].r;
		g = w[0] * buf[i].g;
		b = w[0] * buf[i].b;
		n = w[0];

		x = (start_x + i) % x_size;

		for( int wi = 1; wi <= radius; wi++){
			wc = w[wi];
			x2 = x - wi;
			if(x2 >= 0){
				r += wc * buf[i - wi].r;
				g += wc * buf[i - wi].g;
				b += wc * buf[i - wi].b;
				n += wc;
			}
			x2 = x + wi;
			if(x2 < x_size){
				int idx = i + wi;
				if( idx < buf_size ){
					r += wc * buf[idx].r;
					g += wc * buf[idx].g;
					b += wc * buf[idx].b;
					n += wc;
				} else if( r_buf != NULL ) {
					r += wc * r_buf[idx % buf_size].r;
					g += wc * r_buf[idx % buf_size].g;
					b += wc * r_buf[idx % buf_size].b;
					n += wc;
				}
			}
		}
		tmp_buf[i].r = r/n;
		tmp_buf[i].g = g/n;
		tmp_buf[i].b = b/n;

	}

	if( l_buf != NULL){
		MPI_Wait( &req[0], &status[0] );
		free(l_buf);
	}
    if( r_buf != NULL){
		MPI_Wait( &req[1], &status[1] );
    	free(r_buf);
	}

	for(int i = 0; i < buf_size; i++){
    	buf[i] = tmp_buf[i];
	}

	free(tmp_buf);

}

void rot_array(pixel *in_data, pixel *out_data, int x_size, int y_size){
	int offset = 0;
	int buf_size = x_size * y_size;

	for(int i = 0; i < buf_size; i++){
    	out_data[i] = in_data[ offset + x_size * (i % y_size) ];
		if( i != 0 && (i % y_size) == 0 ){
        	offset++;
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

		check_alloc(data, "data");

		read_ppm( img_path, &x_size, &y_size, &max, (char *) data);

		int size = x_size * y_size;
		int chunk_size = size / p_tot;
		int last_chunk_pad = size - chunk_size * p_tot;

		displs = (int *)malloc(p_tot*sizeof(int));
		check_alloc(displs, "displs");

		scounts = (int *)malloc(p_tot*sizeof(int));

		check_alloc(scounts, "scounts");

		for( int i = 0; i < p_tot; i++){
			displs[i] = i*chunk_size*3;
			scounts[i] = chunk_size*3;
		}

		scounts[p_tot-1] = (chunk_size + last_chunk_pad)*3;

	}

	MPI_Bcast( &x_size, 1, MPI_INT, ROOT_ID, comm );
	MPI_Bcast( &y_size, 1, MPI_INT, ROOT_ID, comm );

	int recv_count;

	MPI_Scatter(scounts, 1, MPI_INT, &recv_count, 1, MPI_INT, ROOT_ID, comm);

	pixel *recv_buf;

	recv_buf = (pixel *)malloc(recv_count*sizeof(char));
	check_alloc(recv_buf, "recv_buf");

	MPI_Scatterv( data, scounts, displs, MPI_CHAR, recv_buf, recv_count, MPI_CHAR,
			ROOT_ID, comm);

    blur_chunk( recv_buf, recv_count, myid, p_tot, x_size );

	MPI_Gatherv( recv_buf, recv_count, MPI_CHAR, data, scounts, displs, MPI_CHAR, ROOT_ID, comm);

	pixel *rot_data;

	if ( myid == ROOT_ID ) {

		rot_data = (pixel *)malloc(MAX_PIXELS*sizeof(pixel));

		check_alloc(rot_data, "rot_data");

		rot_array( data, rot_data, x_size, y_size );
	}

	MPI_Scatterv( rot_data, scounts, displs, MPI_CHAR, recv_buf, recv_count, MPI_CHAR,
			ROOT_ID, comm);

    blur_chunk( recv_buf, recv_count, myid, p_tot, y_size );

	MPI_Gatherv( recv_buf, recv_count, MPI_CHAR, rot_data, scounts, displs, MPI_CHAR, ROOT_ID, comm);

	free(recv_buf);

	if( myid == ROOT_ID ){
		//printf("Totsum: %d\n", tot_sum);
		rot_array( rot_data, data, y_size, x_size );
		write_ppm( "./out.ppm", x_size, y_size, (char *) data);

		free(data);
		free(rot_data);
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

