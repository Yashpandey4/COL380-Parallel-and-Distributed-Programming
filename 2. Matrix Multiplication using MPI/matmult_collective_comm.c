#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>
#include <math.h>
#define forn(i,n) for(int i=0; i < n; i++)
#define THIRTY_TWO 32

int IsEqual(float *A, float *B, int row, int col)
{
/**
 *  A, B are row major matrices of the order row x col
 *  Returns 1 if A and B are identical Matrices
 */
    int i, j;
    int flag = 1;
    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++) {
            if (A[i*col + j] != B[i*col + j])
            {
                flag = 0;
                break;
            }
        }
    }
    return flag;
}

void Matrix_Multiply(float *A, float *B, float *C, int m1, int n1, int p1)
{
/** A is a row major matrices of the order m1 x n1
 *  B is a row major matrices of the order n1 X p1
 *  C is a row major matrices of the order m1 X p1
 *  Function multiplies A and B and stores the result in C
 */
    printf("hii\n");
    int i, j, k;
    for (i = 0; i < m1; i++){
        // printf("%d\n",i);
        for (j = 0; j < p1; j++){
            C[i*p1 + j] = 0.0;
            for (k = 0; k < n1; k++){
                C[i*p1 + j] += A[i*n1 + k] * B[k*p1 + j];
            }
        }
    }
     printf("done\n");
}

void print_error(int n, float*A, float*B, float*C){
    float err=0.0,D=0.0;
    forn(i,n)
        forn(j,n){
            D=0.0;
            forn(k,THIRTY_TWO)
                D += A[i*THIRTY_TWO + k] * B[k*n + j];
             err+=fabsf(D-C[i*n+j]);
        }
        printf("error is %.14lf\n",err);
}

void print_matrix(float **A, int row, int col)
{
    printf("**************************************\n");
    for (int i = 0; i < row; i++)
    {
        printf("\n");
        for (int j = 0; j < col; j++)
        {
            printf("%6.3f ", A[i][j]);
        }
    }
    printf("\n************************************\n");
}

float * create_array(int n, int m)
{                           // allocating memory of n x n to matrix, initializing with 0's
    float *mat = (float*)malloc(n * m*sizeof(float));  
    return mat  ;         
}

int main(int argc, char **argv)
{
	int n = 500;  	// matrix dimension (n x n for output matrix AB)
	if(argc > 1)
		n = atoi(argv[1]);

	printf("n=%d\n",n);

	int rank, size;
	double start,end;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Status status;
    int row_partition = n/size, extra_rows = n%size;

    float  *A,  /* First matrix to multiply */
           *B,  /* Second matrix to multiply */
           *C,  /* To store the result */
           *locA=create_array(row_partition,THIRTY_TWO), /* chunk of A sent to this process */
           *locC=create_array(row_partition,n); /* chunk of C sent to this process */

	
	if (rank == 0) {
        start = MPI_Wtime();
		A=create_array(n,THIRTY_TWO); B=create_array(THIRTY_TWO,n); C=create_array(n,n);

        /* MASTER initializes A and B and puts slave to work */
        forn(i,n)
            forn(j,THIRTY_TWO)
                A[i*THIRTY_TWO+j] = rand() / (float) RAND_MAX;

        forn(i,THIRTY_TWO)
            forn(j,n)
                B[i*n+j] = rand() / (float) RAND_MAX;
        start = MPI_Wtime();
        // printf("starting\n");
        MPI_Scatter(A, THIRTY_TWO * row_partition, MPI_INT,locA, THIRTY_TWO * row_partition, MPI_INT, 0, MPI_COMM_WORLD);
        // printf("scatter complete\n");
		MPI_Bcast(B, THIRTY_TWO*n , MPI_INT ,1, MPI_COMM_WORLD );
		// printf("Broadcast complete\n");
        MPI_Gather(locC, n * row_partition , MPI_INT, C, n * row_partition , MPI_INT , 0, MPI_COMM_WORLD ) ;
        // printf("gather complete\n");
        // Matrix_Multiply(A+32*(size*row_partition),B,C+32*(size*row_partition),extra_rows,THIRTY_TWO,n);
        double end = MPI_Wtime();
        double time = end - start;
        // printf (" ********** time = %0.4fs \n",time);
        print_error(n,A,B,C);
	} 
	else {
        B=create_array(THIRTY_TWO,n);
        // printf("receiving\n");
		MPI_Scatter(A, THIRTY_TWO * row_partition, MPI_INT, locA, THIRTY_TWO * row_partition , MPI_INT, 0, MPI_COMM_WORLD);
        // printf("scatter received\n");
        MPI_Bcast(B, THIRTY_TWO*n , MPI_INT ,1, MPI_COMM_WORLD );
        // printf("Broadcast received\n");
        Matrix_Multiply(locA,B,locC,row_partition,THIRTY_TWO,n);
         // printf("!!!!!!!!multi[ply] complete\n");
        MPI_Gather( locC , n * row_partition , MPI_INT, C, n * row_partition , MPI_INT , 0, MPI_COMM_WORLD ) ;
         // printf("gather sent\n");
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
}