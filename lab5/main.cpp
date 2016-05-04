#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <list>
#include <vector>
#include <iterator>
#include <random>
#include "physics.h"
#include "definitions.h"

static cord_t wall;
const std::size_t particle_size = sizeof(particle_t);
static int nr_of_particles, problem_size;

enum e_direction {
  None = -1,
  N = 0,
  NE,
  E,
  SE,
  S,
  SW,
  W,
  NW
};

e_direction outside_boundry(pcord_t* cord, cord_t boundry) {
  // am I outside of the big box?
  if(cord->x < wall.x0 || cord->x > wall.x1
     || cord->y < wall.y0 || cord->y > wall.y1){
    return e_direction::None;
  }
  // am I going to be outside of my boundry?
  if (cord->x < boundry.x0){
    if (cord->y < boundry.y0) {
      return e_direction::NW;
    }
    else if (cord->y > boundry.y1) {
      return e_direction::SW;
    }
    return e_direction::W;
  }
  else if (cord->x > boundry.x1){
    if (cord->y < boundry.y0) {
      return e_direction::NE;
    }
    else if (cord->y > boundry.y1) {
      return e_direction::SE;
    }
    return e_direction::E;
  }
  else if (cord->y < boundry.y0){
    return e_direction::N;
  }
  else if (cord->y > boundry.y1){
    return e_direction::S;
  }
  return e_direction::None;
}

void part_sim(MPI_Comm comm, int p_tot, int myid, int x_size, int y_size ){
	std::list<particle_t> part_list;

	float x_chunk, y_chunk;
	int coo[2];

	x_chunk = problem_size / static_cast<float>(x_size);
	y_chunk = problem_size / static_cast<float>(y_size);

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

	for( int i = 0; i < nr_of_particles/(x_size*y_size); i++ ){
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
	int iterations = 1000;
	float col_ret;
	float time_step = STEP_SIZE;

	// Allocate a list for managing particles to communicate
	std::vector<std::vector<particle_t>> send_list(8);

	// Allocate an array for which neighbors we got
	std::vector<int> cpu_id_list(8);
	int dest_off_x,dest_off_y;
	for (int i = 0; i < 8; ++i) {
		switch (i) {
			case 0: // North
				dest_off_x = 0;
				dest_off_y = -1;
				break;
			case 1: // NorthEast
				dest_off_x = 1;
				dest_off_y = -1;
				break;
			case 2: // East
				dest_off_x = 1;
				dest_off_y = 0;
				break;
			case 3: // SouthEast
				dest_off_x = 1;
				dest_off_y = 1;
				break;
			case 4: // South
				dest_off_x = 0;
				dest_off_y = 1;
				break;
			case 5: // SouthWest
				dest_off_x = -1;
				dest_off_y = 1;
				break;
			case 6: // West
				dest_off_x = -1;
				dest_off_y = 0;
				break;
			case 7: // NorthWest
				dest_off_x = -1;
				dest_off_y = -1;
				break;
			default:
				printf("Error init cpu_id_list. This should not happen!\n");
		}
		int temp_coo[] = { coo[0] + dest_off_x, coo[1] + dest_off_y };
		if( temp_coo[0] >= 0 && temp_coo[0] < x_size &&
			temp_coo[1] >= 0 && temp_coo[1] < y_size){
			int neighbor_id;
			MPI_Cart_rank( comm, temp_coo, &neighbor_id );
			cpu_id_list[i] = neighbor_id;
		} else {
			cpu_id_list[i] = -1;
		}
	}

	// Boundries for our scope
	cord_t boundry = {start_x,start_x+x_chunk,start_y,start_y+y_chunk};

	float momentum = 0;
	// variable for handling which direction we should communicate
	e_direction dir;
	// For keeping track of vector sizes that we will send later
	int v_size[8];

	for( int i = 0; i < iterations; i++ ){
	  // if a particle is outside the boundry, give it away
	  for( std::list<particle_t>::iterator it = part_list.begin(); it != part_list.end(); ++it ){
		  dir = outside_boundry(&it->pcord, boundry);
		  if (dir != None) {
			  send_list[dir].push_back(*it);
			  it = part_list.erase(it);
		  }
	  }

	  std::vector<MPI_Request> reqs;
	  //reqs.resize(0);
	  // distribute the send_list
	  for (int t = 0; t < send_list.size(); ++t) {
		  if (cpu_id_list[t] > -1) {
			  reqs.resize( reqs.size() + 1 );
			  v_size[t] = send_list[t].size();
        //printf("my id: %d, before send1, t: %d, cpu_id_list: %d\n",myid, t, cpu_id_list[t]);
			  MPI_Isend(&v_size[t], 1,
					  MPI_INT, cpu_id_list[t], 0, comm, &reqs[ reqs.size() -1 ]);
        //printf("my id: %d, after send1, t: %d, cpu_id_list: %d\n",myid, t, cpu_id_list[t]);
			  if (send_list[t].size() > 0 && !send_list[t].empty()) {
				  reqs.resize( reqs.size() + 1 );
          //printf("my id: %d, before send2, t: %d, cpu_id_list: %d\n",myid, t, cpu_id_list[t]);
				  MPI_Isend(&send_list[t][0], particle_size*send_list[t].size(),
						  MPI_CHAR, cpu_id_list[t], 1, comm, &reqs[ reqs.size() -1 ]);
          //printf("my id: %d, after send2, t: %d, cpu_id_list: %d\n",myid, t, cpu_id_list[t]);
			  }
		  }
	  }
    //printf("my id: %d, send_list_size(): %d\n",myid, send_list.size());
	  // Get new particles (if any) from the neighbor threads
	  for (int t = 0; t < cpu_id_list.size(); ++t){
		  if (cpu_id_list[t] > -1) {
			  int recv_size = 0;
			  std::vector<particle_t> temp_vec;
        //printf("my id: %d, before recv1, t: %d, cpu_id_list: %d\n",myid, t, cpu_id_list[t]);
			  MPI_Recv(&recv_size, 1, MPI_INT, cpu_id_list[t], 0, comm, MPI_STATUS_IGNORE);
        //printf("my id: %d, after recv1\n",myid);
			  if( recv_size > 0 ){
				  temp_vec.resize( recv_size );
          //printf("my id: %d, before recv2, temp_vec.size(): %d, recv_size: %d, particle size: %d\n",
          //       myid, temp_vec.size(), recv_size, particle_size);
				  MPI_Recv(&temp_vec[0], recv_size*particle_size, MPI_CHAR, cpu_id_list[t], 1, comm, MPI_STATUS_IGNORE);
          //printf("my id: %d, after recv2, temp_vec.size(): %d\n", myid,temp_vec.size());
				  part_list.insert( std::end(part_list), std::begin(temp_vec), std::end(temp_vec) );
			  }
		  }
	  }

	  if( !reqs.empty() ){
      //printf("my id: %d, before wait, reqs.size(): %d\n",myid,reqs.size());
		  MPI_Waitall(reqs.size(), &reqs[0], MPI_STATUSES_IGNORE);
      //printf("my id: %d, after wait\n",myid);
		  send_list.clear();
      send_list.resize(8);
	  }
    //printf("my id: %d, starting simulation\n", myid);
	  // Simulate our particles
	  for( std::list<particle_t>::iterator it = part_list.begin(); it != part_list.end(); ++it ){
		  for( std::list<particle_t>::iterator it2 = std::next(it); it2 != part_list.end(); ++it2 ){

			  //Check for collisions
			  col_ret = collide(&(it->pcord),&(it2->pcord));

			  if( col_ret != -1.0f ) {
				  interact( &(it->pcord),&(it2->pcord), col_ret );
				  // It is statistically very unlikely that this praticle will collide more than once per time step.
				  // So we skip checking for more collisions.
				  break;
			  }
		  }

		  if( col_ret == -1.0f ){
			  //Move non-collided particles
			  feuler( &(it->pcord), time_step );
		  }
		  //Wall collision check
		  momentum += wall_collide( &(it->pcord), wall );
	  }
  }

  // Simulation is done, gather the total momentum sum
  float momentum_sum = 0;

  MPI_Allreduce(&momentum, &momentum_sum, 1, MPI_FLOAT, MPI_SUM, comm);

  if( myid == 0 ){
	  float presure = momentum_sum / (static_cast<float>(iterations));
	  //presure = presure / WALL_LENGTH;
	  printf( "Presure: %f\n", presure );
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
	return 1;
}

int main(int argc, char const* argv[])
{

	if( argc != 5 ){
		printf("Not enough arguments, need 4:\n\t<x> <y> <particles> <problem size> \n");
		return -1;
	}

	int x_size, y_size;

	x_size = atoi(argv[1]);
	y_size = atoi(argv[2]);
  nr_of_particles = atoi(argv[3]);
  problem_size = atoi(argv[4]);

  wall = {0.0f, static_cast<float>(problem_size), 0.0f, static_cast<float>(problem_size)};
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
