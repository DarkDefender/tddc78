#include <mpi.h>
#include <stdio.h>

int main( void )
{
	MPI_Status status;
	char string[5];  // receive buffer
	int myid;
	MPI_Init( NULL, NULL );
	MPI_Comm_rank( MPI_COMM_WORLD, &myid );
	if (myid==2)
		MPI_Send( "HELLO", 5, MPI_CHAR, 3, 1234, MPI_COMM_WORLD );
	if (myid==3) {
		MPI_Recv( string, 5, MPI_CHAR, 2, MPI_ANY_TAG,
				MPI_COMM_WORLD, &status );
		printf( "Got %s from P%d, tag %d\n",
				string, status.MPI_SOURCE, status.MPI_TAG );
	}
	MPI_Finalize();
	return 0;
}
