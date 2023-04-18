#include <mpi.h>
#include <sprng.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "NeutronTransport.h"
#include "sprng_cpp.h"	


#define reportsInterval 1000
#define requiredParams 5
#define dataFileName "./out/results.dat"
#define LCG 0
#define SEED 12345678

int main(int argc, char** argv) 
{
    NeutronTransportParam params{};
    unsigned long n;

    int my_id;
    int masterId;
    int nproc;
    unsigned i, totalIterations, captured, passed, reflected;
    unsigned totalCaptured, totalPassed, totalReflected;
    double chanceCapture, chancePass, chanceReflected;
    Sprng *stream;

    //Sprng *stream;

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
        printf("Solving the problem of transport to neutrons using the monte carlo method\n");
        printf("Cc\t=\t%lf\n",  params.Cc );
        printf("Cs\t=\t%lf\n",  params.Cs );
        printf("C\t=\t%lf\n",   params.C );
        printf("H\t=\t%lf\n\n", params.H );
    }



    ///////////////////////
    stream = SelectType(LCG);
    stream->init_sprng(my_id,nproc, SEED, SPRNG_DEFAULT);


////////////
    MPI_Bcast(&params, 1, getMpiNeutronTransportParam(), 0, MPI_COMM_WORLD);
    n = params.N / (nproc);

    reflected = passed = captured = 0;

    for (i = 0; i < n; ++i)
    {
        switch (singleNeutronTransport(&params, stream))
        {
        case Result::Passed:
            ++passed;
            break;
        case Result::Reflected:
            ++reflected;
            break;
        case Result::Captured:
            ++captured;
            break;
        
        default:
            break;
        }

        if (i % reportsInterval == 0)
        {
            MPI_Allreduce(&i, &totalIterations, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);

            if (my_id == masterId)
            {
                printf("\rCurrent iteration = %u/%u", totalIterations, params.N);
            }
        }
    }

    MPI_Allreduce(&reflected, &totalReflected, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&captured, &totalCaptured, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&passed, &totalPassed, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&i, &totalIterations, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);

    if (my_id == masterId)
    {
        chanceReflected = double(totalReflected)/totalIterations;
        chanceCapture   = double(totalCaptured)/totalIterations;
        chancePass      = double(totalPassed)/totalIterations;
        printf("\n\nComputation finished!\n\n");
        printf("Number of iterations: %u\n", totalIterations);
        printf("Chance to capture neutron: %.23lf\n", chanceCapture);
        printf("Chance to neutron pass: %.23lf\n", chancePass);
        printf("Chance to reflect neutron: %.23lf\n", chanceReflected);
        writeResultDataToFile(&params, dataFileName, chanceCapture, chancePass, chanceReflected, totalIterations);
    }

    //free_sprng(stream);
    MPI_Finalize();

    return 0;
}