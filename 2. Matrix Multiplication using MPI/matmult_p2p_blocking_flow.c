#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define forn(i,n) for(int i=0; i < n; i++)
#define MASTER 0  /* PID of Master process */
#define MASTER_TO_SLAVE_TAG 0
#define SLAVE_TO_MASTER_TAG 1
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
    int i, j, k;
    for (i = 0; i < m1; i++){
        for (j = 0; j < p1; j++){
            C[i*p1 + j] = 0.0;
            for (k = 0; k < n1; k++)
                C[i*p1 + j] += A[i*n1 + k] * B[k*p1 + j];
        }
    }
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

float *create_array(int n, int m)
{                           // allocating memory of n x n to matrix, initializing with 0's
    float *mat = (float*)malloc(n * m*sizeof(float));  
    return mat  ;         
}

void print_error(int n, float*A, float*B, float*C){
    float *D=create_array(n,n);
        forn(i,n)
            forn(j,n)
                forn(k,THIRTY_TWO)
                    D[i*n + j] += A[i*THIRTY_TWO + k] * B[k*n + j];

        float err=0.0;
        forn(i,n)
            forn(j,n)
                err+=fabsf(D[i*n+j]-C[i*n+j]);

        printf("error is %.14lf\n",err);
}

int main (int argc, char *argv[])
{
    int n = 40;    // matrix dimension (n x n for output matrix AB)
    if(argc > 1)
        n = atoi(argv[1]);

    printf("n=%d\n",n);

    float  *A,  /* First matrix to multiply */
           *B,  /* Second matrix to multiply */
           *C;  /* To store the result */

    int rank, size, tag;
    int row_partition, first, extra_rows, total_elems, send_rows;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;

    if(size <= 1)
    {
        MPI_Abort(MPI_COMM_WORLD, 0);
        exit(1);
    }

    if(rank == MASTER)
    {
        A = create_array(n,THIRTY_TWO);
        B = create_array(THIRTY_TWO,n);
        C = create_array(n,n);

        srand((unsigned int)time(NULL));

        /* MASTER initializes A and B and puts slave to work */
        forn(i,n)
            forn(j,THIRTY_TWO)
                A[i*THIRTY_TWO+j] = rand() / (float) RAND_MAX;

        forn(i,THIRTY_TWO)
            forn(j,n)
                B[i*n+j] = rand() / (float) RAND_MAX;

        row_partition = n/size;
        extra_rows = n%size;
        tag = MASTER_TO_SLAVE_TAG;
        total_elems = 0;
        double start = MPI_Wtime();
        /* send info from master to slave */
        for(int i=0; i<size; i++)
        {
            send_rows = row_partition;
            if(i<extra_rows)
                send_rows += 1;
            if(i==0){
                first=send_rows;
                total_elems += send_rows;
                continue;
            }

            MPI_Send(&total_elems, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
            MPI_Send(&send_rows, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
            MPI_Send(A+THIRTY_TWO*total_elems, send_rows*THIRTY_TWO, MPI_FLOAT, i, tag, MPI_COMM_WORLD);
            MPI_Send(B, THIRTY_TWO*n, MPI_FLOAT, i, tag, MPI_COMM_WORLD);

            total_elems += send_rows;
        }

        forn(i,first)
            forn(j,n)
                forn(k,THIRTY_TWO)
                    C[i*n+j] += A[i*THIRTY_TWO+k]*B[k*n+j];

        /* receive info from slaves */
        tag = SLAVE_TO_MASTER_TAG;

        for(int i=1; i<size; i++)
        {
            MPI_Recv(&total_elems, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(&send_rows, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(C+n*total_elems, send_rows*n, MPI_FLOAT, i, tag, MPI_COMM_WORLD, &status);
        }

        double end = MPI_Wtime();
        double time = end - start;
    
        print_error(n,A,B,C);

        printf("The time taken for Matrix Multiplication of Sizes %dx%d and  %dx%d using Blocking P2P Communication is %0.4fs\n",n,THIRTY_TWO,THIRTY_TWO,n, time);
    }

    if(rank > MASTER)
    {
        /* Slaves Multiply smaller parts of A with B */

        tag = MASTER_TO_SLAVE_TAG;
        MPI_Recv(&total_elems, 1, MPI_INT, MASTER, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&send_rows, 1, MPI_INT, MASTER, tag, MPI_COMM_WORLD, &status);

        A = create_array(send_rows,THIRTY_TWO);
        B = create_array(THIRTY_TWO,n);
        C = create_array(send_rows,n);

        MPI_Recv(A, send_rows*THIRTY_TWO, MPI_FLOAT, MASTER, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(B, THIRTY_TWO*n, MPI_FLOAT, MASTER, tag, MPI_COMM_WORLD, &status);

        forn(i,send_rows)
            forn(j,n)
                forn(k,THIRTY_TWO)
                    C[i*n+j] += A[i*THIRTY_TWO+k]*B[k*n+j];

        tag = SLAVE_TO_MASTER_TAG;
        MPI_Send(&total_elems, 1, MPI_INT, MASTER, tag, MPI_COMM_WORLD);
        MPI_Send(&send_rows, 1, MPI_INT, MASTER, tag, MPI_COMM_WORLD);
        MPI_Send(C,send_rows*n, MPI_FLOAT, MASTER, tag, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}
