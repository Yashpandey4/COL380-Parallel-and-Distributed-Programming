#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

        int n , pid, p ;

	// sendA , sendB are data in p0
	// finalC is gathered by p0
	// recA and recC are local data in all other processors
        int *sendA, *sendB, *finalC , *recA , *recC ;

        int tagn = 0 ;
        int tagA = 1 ;
        int tagB = 2 ;
        int tagC = 3 ;

        char processor_name[MPI_MAX_PROCESSOR_NAME];

        MPI_Init(&argc,&argv);

        MPI_Comm_size(MPI_COMM_WORLD, &p); // max number of processors
        MPI_Comm_rank(MPI_COMM_WORLD, &pid); // our rank

        MPI_Status status;

        MPI_Comm comm1 , comm2;

	// p0 is reading the file
        if (pid ==0 ) {

                FILE *fp = fopen("data.txt","r");

                fscanf ( fp , "%d", & n );
                printf ("rows is %d \n", n );

                sendA = (int*) malloc( n *n * sizeof( int* ));
                sendB = (int*) malloc( n *n *sizeof( int* ));
                finalC = (int*) malloc( n *n* sizeof( int* ));


                for ( int r = 0 ; r < n ; r++)
                        for ( int c = 0 ; c < n ; c++ )
                        {
                                fscanf ( fp , "%d", & sendA[r*n + c] );
                                //printf ("A [%d] %d \n", r*n + c , sendA[r*n + c] );
                        }
                for ( int r = 0 ; r < n ; r++)
                        for ( int c = 0 ; c < n ; c++ )
                                fscanf ( fp , "%d", & sendB[r*n + c] );


                fclose (fp);
                printf (" process 0 send create buffer \n " ) ;

        }


	// this part is added to start timing , all processors are doing this part
	// I decide that all process calculate the time
	// I was curious to see what happened

	MPI_Barrier(MPI_COMM_WORLD); /* barrier is needed if no necessary synchronization for the timing is ensured yet */
	double start = MPI_Wtime(); /* only 1 process needs to do this */
		
        // first broad cast the n to all others
        MPI_Bcast ( &n, 1 , MPI_INT , 0 , MPI_COMM_WORLD ) ;
        printf ("  n / p is %d \n " , n/p  ) ;


	// allocating lical data
        recA = (int*) malloc( n/p * n * sizeof( int* ));
	if (sendB == NULL ) { printf ("NULL\n"); sendB = (int*) malloc( n * n* sizeof( int* ));}
        recC = (int*) malloc( n /p *n * sizeof( int* ));
        printf (" process %d create rec buffer \n " , pid  ) ;


	// now p0 scatter matrix sendA to all 
        MPI_Scatter( sendA , n * n/p , MPI_INT , recA , n * n/p , MPI_INT , 0, MPI_COMM_WORLD ) ;

        for (int i=0; i < (n/p) ; i++)
            printf (" process %d A [%d] = % d \n " , pid , i , recA[i] ) ;
        

        //now p0 broadcast sendB to all others
        MPI_Bcast ( sendB, n*n , MPI_INT , 0 , MPI_COMM_WORLD ) ;

        for (int i=0; i < (n*n) ; i++)
		printf (" process %d sendB [%d] = % d \n " , pid , i , sendB[i] ) ;
 		

	// all do this part to calculate recC as multiplicated matrix		
        for ( int i=0 ; i< (n/p) ; i++ )
                for ( int j = 0 ; j<n ; j++ )
                {
                        recC[i*n+j] = 0 ;
                        for ( int k = 0 ; k<n ; k++ )
                                recC[i*n+j] += recA[i*n+k]*sendB[k*n+j];
						printf (" process %d recC [%d] = % d \n" , pid , i*n+j , recC[i*n+j] ) ;
                        
				}
		
	// nopw p0 will gather all result data from all prcesses
        MPI_Gather( recC , n*n/p , MPI_INT , finalC , n*n/p , MPI_INT , 0, MPI_COMM_WORLD ) ;


	// here is the last point of calculating the time		
	MPI_Barrier(MPI_COMM_WORLD); /* barrier is needed if no necessary synchronization for the timing is ensured yet */
	double end = MPI_Wtime(); /* only 1 process needs to do this */

	// we write the time
	double time = end - start ;
	printf (" ********** process %d : time = % d \n" , pid , time ) ;
		
	
	// here p0 is writing the output into the file	
        if (pid == 0) {
		
			FILE *out = fopen("matrix_result.txt","w");
			if (out == NULL)
			{
					printf("Error opening file!\n");
					exit(1);
			}

			for (int i=0; i < n ; i++)
			{
				for (int j=0; j < n ; j++)
					fprintf ( out ," %d " , finalC[i*n+j] ) ;
				fprintf ( out , "\n") ;	
			}
			fclose ( out) ;
		}


	// thank u professor song jiang for reading our code precisely
	// I really appreciate that

        MPI_Finalize();

	return 0;
}