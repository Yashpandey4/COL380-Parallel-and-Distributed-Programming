#include <bits/stdc++.h>
#include <omp.h>

using namespace std;
using namespace chrono;

#define forn(i,n) for(int i=0; i < n; i++)

int n, num_thread;
double delta = 0.000000001;                            // very small value

void initialize_A(double** A, double** Areal)
{                                                      // initializing the input matrix, making a copy for future reference
    forn(i,n)
        forn(j,n){
            A[i][j] = drand48() * 100;
            Areal[i][j] = A[i][j];                     // Keeping a copy of the original matrix (before its modified)
        }
}

void initialize_U(double** U)
{                                                      // initializing the upper triangular matrix
    forn(i,n)
        forn(j,n)
            if(i <= j)
                U[i][j] = drand48() * 100;
}

void initialize_L(double** L)
{                                                      // initializing the lower triangular matrix
    forn(i,n)
        forn(j,n)
            if(i > j)
                L[i][j] = drand48() * 100;
            else if(i == j)
                L[i][j] = 1.0;
}

void print_matrix(double** matrix)
{                                                     // printing given matrix
    forn(i,n){
        forn(j,n)
            cout << matrix[i][j] << " ";
        cout << endl;
    }
    cout << "--------------------------" << endl;
}

double **create_matrix()
{
    double **m = (double**)malloc(n * sizeof(double*));             
    forn(i,n){
        m[i] = (double*)malloc(n * sizeof(double));  // allocating memory of n x n to matrix, initializing with 0's
        forn(j, n)
            m[i][j] = 0.0;
    }

    return m;
}

double calculate_residue(double** P, double** A, double** L, double** U)
{                                                   // returns the variance of matrix ( PA - LU ) for finding error magnitude
    double res = 0.0, res1;

    forn(i,n)
        forn(j,n){
            res1= 0.0;
            forn(k,n)
                res1 += P[i][k] * A[k][j] - L[i][k] * U[k][j] ;
            res += res1 * res1;
        }

    return res;
}

void freeMemory(double** matrix)
{                                                   // deleting all variables with significant memory size
    forn(i,n)
        delete[] matrix[i];
    delete[] matrix;
}

void lu_decomposition()
{

    int* pi = (int*)malloc(n * sizeof(int));
    double** P = create_matrix(),**A = create_matrix(),
        **Areal = create_matrix(),**U = create_matrix(),**L = create_matrix();

    initialize_A(A,Areal);
    initialize_U(U);
    initialize_L(L);

    auto t1 = high_resolution_clock::now();        // starting timer

    forn(i,n)
        pi[i] = i;                                 // initialize pi as a vector of length n

    
    forn(k,n)
    {
        double max_A = 0.0;                        // max value below (including) main diagonal in k-th row of A    * PIVOT *
        int k1 = -1;                               // row index of this max value

        for(int i=k; i < n; i++)
            if(max_A < abs(A[i][k]))
            {
                max_A = abs(A[i][k]);
                k1 = i;
            }

        if(abs(max_A)<delta)                       // raise an error for the matrix being singular
            perror("received singular matrix A");

        swap(pi[k], pi[k1]);
        swap(A[k], A[k1]);                         // swap k-th row with k1-th

        forn(m,k)
            swap(L[k][m], L[k1][m]);

        U[k][k] = A[k][k];

        for(int i=k+1; i < n; i++)
        {
            L[i][k] = A[i][k]/(U[k][k]+delta);     // dividing A(k+1:n,k) by pivot and copying to L(k+1:n,k)
            U[k][i] = A[k][i];                     // copying A(k,k+1:n) to U(k,k+1:n)
        }
        int i;

        # pragma omp parallel for num_threads(num_thread) private(i) shared(A,L,U)
                                                   // parallelized the loop keeping i private (unchanged when exiting omp block) sharing matrices A,L and U
            for(i=k+1; i < n; i++)
            {
                for(int j=k+1; j < n; j++)
                {
                    A[i][j] -= L[i][k]*U[k][j];   // decrementing A(k+1:n,k+1:n) by [ L(k+1:n,k) ]*[ U(k,k+1:n) ]'
                }
            }
    }

    forn(i,n)
        P[i][pi[i]] = 1.0;                        // converting pi into a 2D array by replacing pi[i] with its one hot embedding.

    auto t2 = high_resolution_clock::now();       // ending timer
    cout << "time taken by LU decomposition: " <<
    duration_cast<microseconds>( t2 - t1 ).count()<<"\n"; // printing the time taken (in seconds)
    
    // cout << calculate_residue(P, Areal, L, U)<<"\n";

    freeMemory(P);                                // deleting all variables with significant memory size
    freeMemory(A);
    freeMemory(Areal);
    freeMemory(L);
    freeMemory(U);
    delete [] pi;
}

int main(int argc, char* argv[])
{
    n = stoi(argv[1]), num_thread = stoi(argv[2]);
    cout << "*********OPENMP***************";
    cout<< n <<" "<< num_thread<<"\n";
    srand48((unsigned int) time(nullptr));        // seed for pseudo-random number generator

    lu_decomposition();


    return 0;
}