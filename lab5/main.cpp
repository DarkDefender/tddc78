#include <mpi.h>
#include <stdio.h>  
#include <stdlib.h>
#include <cmath>
#include <list>
#include <random>
#include "physics.h"
#include "definitions.h"

void part_sim(MPI_Comm comm, int p_tot, int myid, int x_size, int y_size ){
	std::list<particle_t> part_list;		

	float x_chunk, y_chunk;
	int coo[2];

	x_chunk = BOX_HORIZ_SIZE / static_cast<float>(x_size); 
	y_chunk = BOX_VERT_SIZE / static_cast<float>(y_size); 

	MPI_Cart_coords( comm, myid, 2, coo );

	float start_x, start_y;

	start_x = static_cast<float>(coo[0]) * x_chunk;
	start_y = static_cast<float>(coo[1]) * y_chunk;

	std::random_device rd;
	std::uniform_real_distribution<float> dist_velo(0,MAX_INITIAL_VELOCITY);
	std::uniform_real_distribution<float> dist_angle(0, 2.0f * PI);

	std::uniform_real_distribution<float> dist_x(start_x, start_x + x_chunk);
	std::uniform_real_distribution<float> dist_y(start_y, start_y + y_chunk);

	float r, phi;

	for( int i = 0; i < INIT_NO_PARTICLES; i++ ){
        particle_t new_part;
        
        r = dist_velo(rd);
		phi = dist_angle(rd);

		new_part.pcord.x = dist_x(rd);
		new_part.pcord.y = dist_y(rd);

		new_part.pcord.vx = r * cos(phi);
		new_part.pcord.vy = r * sin(phi);

    	part_list.push_back(new_part);
	}

	//Total number of iterations to do
    int iterations = 100; 

	for( int i = 0; i < iterations; i++ ){
    	for( std::list<particle_t>::iterator it = part_list.begin(); it != part_list.end(); ++it ){

			//Check for collisions

			//Move non-collided particles
			
			//Wall collision check

		}
	}

}

//Setup the MPI CPU chart
int init_cpu_cart(MPI_Comm *new_comm,int myid, int p_tot, int x_size, int y_size){

    if( x_size * y_size != p_tot ){
    	printf("The number of threads/cpus fit in the specified grid size\n");
		return 0;
	}

	MPI_Comm comm = MPI_COMM_WORLD;

	int dims[2], coo[2], period[2], src, dest;
	period[0]=period[1]=0; // 0=grid, !0=torus
	int reorder=0; // 0=use ranks in communicator,
			   // !0=MPI uses hardware topology
	dims[0] = x_size; // extents of a virtual
	dims[1] = y_size; // processor grid

	// create virtual 2D grid topology:
	MPI_Cart_create( comm, 2, dims, period,
			reorder, new_comm );
    /*
	// get my coordinates in 2D grid:
	MPI_Cart_coords( new_comm, myid, 2, coo );

	// get rank of my grid neighbor in dim. 0
	MPI_Cart_shift( new_comm, 0, +1, // to south,
			&src, &dest); // from south

	printf("%d: My new id is: %d, %d\n", myid, coo[0], coo[1]);
	printf("%d Src: %d, dest: %d\n", myid, src, dest);
	*/
	return 1;
}

int main(int argc, char const* argv[])
{
	
	if( argc != 3 ){
		printf("Problem grid size\n");
		return -1;
	}

	int x_size, y_size;

	x_size = atoi(argv[1]);
	y_size = atoi(argv[2]);
    
	int myid, p_tot;
	MPI_Init( NULL, NULL );
	MPI_Comm_rank( MPI_COMM_WORLD, &myid );
	MPI_Comm_size( MPI_COMM_WORLD, &p_tot );

	MPI_Comm new_comm;

	if ( init_cpu_cart( &new_comm, myid, p_tot, x_size, y_size ) ){
		double starttime, endtime;
		starttime = MPI_Wtime();

		part_sim( new_comm, p_tot, myid, x_size, y_size );

		endtime = MPI_Wtime();
		printf("That took %f seconds on id:%d\n", endtime-starttime, myid );
	}

	MPI_Finalize();
	return 0;
}
