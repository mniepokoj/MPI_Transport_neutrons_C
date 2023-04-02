#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define M_PI 3.141592653589793
#define REQUEST   1
#define REPLY     2
#define CHUNKSIZE 16384

double drand()
{
    return ((double)rand() / (RAND_MAX)) + 1;
};

double calculateDistance(double C, double u)
{
    return -(1.) / C * log(u);
}

int singleNeutronTransport(double C, double Cc, double H)
{
    double neutron_pos = 0;

    double u = drand();
    double L = calculateDistance(C, u);
    double D = drand() * M_PI;
    neutron_pos += L * cos(D);
    int hasPassed;

    while (true)
    {
        double absorbedValue = drand() * C;
        if (neutron_pos <= 0 || absorbedValue <= Cc)
        {
            return 0;
        }
        if (neutron_pos >= H)
        {
            return 1;
        }

        u = drand();
        L = calculateDistance(C, u);
        D = drand() * M_PI;
        neutron_pos += L * cos(D);
    }
}

double neutronTransport(unsigned long N, double C, double Cc, double H)
{
    long tries = 0;
    int passed = 0;
    for (int i = 0; i < N; i++)
    {
        int hasTransmitted = singleNeutronTransport(C, Cc, H);

        tries += 1;
        passed += hasTransmitted;
    }

    return (double)passed / tries;
}

int main(int argc, char** argv) 
{
    double Cc = 0.3;
    double Cs = 0.7;
    double C = 1;
    double H = 1;
    unsigned long N = 2000000000, n;
    double result;

    int my_id;
    int serverId;
    int nproc;

    MPI_Comm world, workers;
    MPI_Group world_group, worker_group;
    MPI_Status status;
    int ranks[1];
    int request;
    int i, iter, done, max, min, workerId;
    unsigned in, out, totalIn, totalOut;
    double* rands = (double*)malloc(CHUNKSIZE * sizeof(double));

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    serverId = nproc - 1;
    n = N / (nproc - 1);

    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    ranks[0] = serverId;
    MPI_Group_excl(world_group, 1, ranks, &worker_group);
    MPI_Comm_create(MPI_COMM_WORLD, worker_group, &workers);
    MPI_Group_free(&worker_group);
    MPI_Group_free(&world_group);

    in = 0;
    out = 0;

    if (my_id == serverId)
    {
        srand(my_id);
        do {
            MPI_Recv(&request, 1, MPI_INT, MPI_ANY_SOURCE, REQUEST,
                MPI_COMM_WORLD, &status);

            if (request)
            {
                for (i = 0; i < CHUNKSIZE; i++)
                {
                    rands[i] = rand();
                }
                MPI_Send(rands, CHUNKSIZE, MPI_DOUBLE,
                    status.MPI_SOURCE, REPLY, MPI_COMM_WORLD);
            }
        } while (request > 0);
    }
    else
    {
        request = 1;

        for (i = 0; i < n;)
        {
            MPI_Send(&request, 1, MPI_INT, serverId, REQUEST, MPI_COMM_WORLD);
            MPI_Recv(rands, CHUNKSIZE, MPI_DOUBLE, serverId, REPLY, MPI_COMM_WORLD, &status);
            for (iter = 0; iter < CHUNKSIZE && i < n; iter++, i++)
            {
                in++;
                out += singleNeutronTransport(C, Cc, H);
            }
        }
        MPI_Allreduce(&in, &totalIn, 1, MPI_UINT32_T, MPI_SUM, workers);
        MPI_Allreduce(&out, &totalOut, 1, MPI_UINT32_T, MPI_SUM, workers);

        if (my_id == 0)
        {
            request = 0;
            MPI_Send(&request, 1, MPI_INT, serverId, REQUEST, MPI_COMM_WORLD);

            printf("wynik to: %.20f\nNa podstawie %d prob", double(totalOut) / totalIn, totalIn);
        }
    }
    MPI_Finalize();
    return 0;
}