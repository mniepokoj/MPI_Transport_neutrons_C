#include <mpi.h>
#include <stdio.h>
int main(int argc, char** argv) 
{
    char* a = argv[0];
    char* b = argv[1];
    char* c = argv[2];
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    // Get the rank of the process
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    // Print the message
    printf("Hello World! My rank is %d\n", my_rank);
    // Finalize the MPI environment.
    MPI_Finalize();
}