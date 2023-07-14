//*******************	Include header files	*******************
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <mpi.h>     /* For MPI functions, etc */ 



//*******************	Define global input and output matrix and parameters 	*******************
double* A;
double* B;
double* C_serial;		// C = A x B
double* C_parallel;		// C = A x B
unsigned int N;



//*******************	Fill in a given matrix with random numbers 	*******************
void fillmatrix(double* data, unsigned int rows)
{
	unsigned int i;
	for (i = 0; i < rows*rows; i++)
		data[i] = (rand()%100) / 12.345;
}



//*******************	Calculate mse for two given arrays 	*******************
double calc_mse (double* data1, double* data2, int size) {
	double mse = 0.0;
	int i;
	double e;
	for (i=0; i<size; i++) {
		e = data1[i]-data2[i];
		e = e * e;
		mse += e;
	}
	return mse;
}



//*******************	Calculate mse for first and last row of final results 	*******************
double testresult(double* data1 , double* data2 , int rows){
	double mse = 0.0;
	mse += calc_mse( data1 , data2, rows );										// first row
	mse += calc_mse( data1 + rows*(rows-1), data2 + rows*(rows-1), rows );		// last row
	return mse;
}



//*******************	Calculate minimum of two values 	*******************
int min2 (int value1 , int value2)
{
	if (value1 < value2)
	{
		return value1;
	}
	else
	{
		return value2;
	}
}



//*******************	Main function	*******************
int main( int argc, char *argv[] )
{
	
//*******************	Variables definition and memory allocation 	*******************
	int				comm_sz;               /* Number of processes    */
	int				my_rank;               /* My process rank        */
	unsigned int	N_rows;
	int				i, j, k;
	int 			active_cores;
	double 			mse;
	
	
	// N = 2 ^ M
	unsigned int M = atoi(argv[1]);
    N = (1 << M);
	//N = (unsigned int) pow (2.0, 10.0);
	
	srand(time(0));

	
	//time_t MPI_t1, MPI_t2, serial_t1, serial_t2;
	clock_t MPI_t1, MPI_t2, serial_t1, serial_t2;
	
	//double MPI_time, serial_time; //t2-t1
	clock_t MPI_time, serial_time;

	
	//input array
	A = (double*) malloc ( sizeof(double) * N * N );
	B = (double*) malloc ( sizeof(double) * N * N );
	C_serial = (double*) malloc ( sizeof(double) * N * N );
	C_parallel = (double*) malloc ( sizeof(double) * N * N );
	
	
	
//*******************	Fill in matrix A and B with random numbers 	*******************
	fillmatrix(A, N);	
	fillmatrix(B, N);	
	
	
	
//*******************	Serial calculation	*******************
	
	//serial_t1 = time(0);
	serial_t1 = clock();
	
	for (i=0; i<N; i++)
	{
		for(j=0; j<N; j++)
		{
			C_serial[i*N+j] = 0;
			for(k=0; k<N; k++)
			{
				C_serial[i*N+j] += A[i*N+k] * B[k*N+j];
			}
		}
	}
	
	//serial_t2 = time(0);
	serial_t2 = clock();
	
	serial_time = serial_t2 - serial_t1;
	
//*******************	End of serial calculation	*******************
	
	
	
//*******************	Parallel calculation with MPI	*******************	

	//t1
	//MPI_t1 = time(0);
	MPI_t1 = clock();
	
	//distribute input data among P processes, 
	//calculate pieces of array C in parallel,
	//and finally merge the result to form C.
	
	/* Start up MPI */
	MPI_Init(NULL, NULL); 

	/* Get the number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); 

	/* Get my rank among all the processes */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 
	
	N_rows = N/comm_sz;
	if(N_rows == 0)
	{
		N_rows = 1;
	}
	
	active_cores = min2 (N/N_rows , comm_sz);
	
	
	if (my_rank == 0)
	{
		for(i=1; i<active_cores; i++)
		{
			MPI_Send(A+ i*N*N_rows, N*N_rows , MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
		}
	}
	else if (my_rank < active_cores)
	{
		MPI_Recv(A, N*N_rows, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	
	
	if (my_rank == 0)
	{
		for(i=1; i<active_cores; i++)
		{
			MPI_Send(B, N*N , MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
		}
	}
	else if (my_rank < active_cores)
	{
		MPI_Recv(B, N*N, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	
	
	
	if (my_rank < active_cores)
	{
		for (i=0; i<N_rows; i++)
		{
			for(j=0; j<N; j++)
			{
				C_parallel[i*N+j] = 0;
				for(k=0; k<N; k++)
				{
					C_parallel[i*N+j] += A[i*N+k] * B[k*N+j];
				}
			}
		}
	}
	
	
	
	if (my_rank == 0)
	{
		for(i=1; i<active_cores; i++)
		{
			MPI_Recv(C_parallel+i*N*N_rows, N*N_rows, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
	}
	else if (my_rank < active_cores)
	{
		MPI_Send(C_parallel, N*N_rows, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
	}
	
	
	
	//t2
	//MPI_t2 = time(0);
	MPI_t2 = clock();

	//t2-t1
	MPI_time = MPI_t2 - MPI_t1;

//*******************	End of parallel calculation with MPI	*******************	
	
	
	
//*******************	Compare results and print ellapsed times and mse 	*******************	
	if (my_rank == 0)
	{
		mse = testresult(C_serial, C_parallel, N);
	
		printf ( "M=%d , N=%d , rank_number=%d , serial_time=%g ms , MPI_time=%g ms , mse=%g\n" , M , N , my_rank , serial_time/1000.0 , MPI_time/1000.0 , mse);
	}
	
	
	/*
	mse = testresult(C_serial, C_parallel, N);
	
	printf ( "M=%d , N=%d , rank_number=%d , serial_time=%g ms , MPI_time=%g ms , mse=%g\n" , M , N , my_rank , serial_time/1000.0 , MPI_time/1000.0 , mse);
	*/
	
	
	
//*******************	Finalizing level	*******************	
	//	Shut down MPI
	MPI_Finalize(); 

	return 0;
}