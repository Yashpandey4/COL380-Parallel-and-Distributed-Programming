#include <stdio.h>
#include "mpi.h"
#include <string.h>
#include <stdlib.h>
void main(int argc, char **argv)
{
	char message[20],message2[20];
	int i, rank, size, tag = 99;
	MPI_Status status;
	double* a[10];
	for(int i=0;i<100000;i++)
		for(int j=0;j<100000;j++){
			int k=0;
		}
		//a[i] = (double*)malloc(100 * sizeof(double));

	printf("made array\n");  
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if (rank == 0) {
		strcpy(message, "Hello, world!");
		strcpy(message2, "WASSUP  BITCHES?");
		for (i = 1; i < size; i++){
			MPI_Send(message, 13, MPI_CHAR, i, tag, MPI_COMM_WORLD);
			MPI_Send(message2, 13, MPI_CHAR, i, tag, MPI_COMM_WORLD);
		}
	} 
	else {
		MPI_Recv(message, 20, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &status);
		MPI_Recv(message2, 20, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &status);
	}
	printf("%d : %.13s", rank, message);
	printf("%d : %.13s\n", rank, message2);
	MPI_Finalize();
}