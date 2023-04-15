#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "NeutronTransport.h"


#define reportsInterval 10000
#define requiredParams 5

int main(int argc, char** argv) 
{
    NeutronTransportParam params{};
    unsigned long n;

    int my_id;
    int masterId;
    int nproc;
    unsigned long i, j, in, out, totalIn, totalOut, iterationTotal;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    masterId = 0;

    if (argc < requiredParams)
    {
        if (my_id == masterId)
        {
            printf("Not enough arguments\n Correct order is NUMBER_OF_COMPUTATIONS Cs Cc H.");
        }
        return 0;
    }

    if (my_id == masterId)
    {
        initNeutronTransportParam(argc, argv, &params);
    }

    MPI_Bcast(&params, 1, getMpiNeutronTransportParam(), 0, MPI_COMM_WORLD);
    n = params.N / (nproc);

    in = out = 0;

    for (i = 0; i < n; i++)
    {
        if (i % reportsInterval == 0)
        {
            MPI_Allreduce(&in, &totalIn, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);
            MPI_Allreduce(&out, &totalOut, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);
            MPI_Allreduce(&i, &iterationTotal, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);

            if (my_id == masterId)
            {
                printf("\rResult = %23.20f\t\tIteration = %ldm/%ldm", double(totalOut) / totalIn, totalIn/1000000, params.N/1000000);
            }
        }
        in++;
        out += singleNeutronTransport(&params);
    }

    MPI_Allreduce(&in, &totalIn, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&out, &totalOut, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);

    if (my_id == masterId)
    {
        printf("wynik to: %.20f\nNa podstawie %d prob", double(totalOut) / totalIn, totalIn);
    }

    MPI_Finalize();

    return 0;
}