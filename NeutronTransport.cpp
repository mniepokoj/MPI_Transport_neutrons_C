#include "NeutronTransport.h"
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#define M_PI 3.141592653589793


int initNeutronTransportParam(int argc, char** argv, NeutronTransportParam* params)
{
    if (argc < 5)
    {
        return -1;
    }

    sscanf_s(argv[1], "%ld", &params->N);
    sscanf_s(argv[2], "%lf", &params->Cc);
    sscanf_s(argv[3], "%lf", &params->Cs);
    sscanf_s(argv[4], "%lf", &params->H);
    params->C = params->Cc + params->Cs;

    return 1;
}


double drand()
{
    return ((double)rand() / (RAND_MAX)) + 1;
};

double calculateDistance(double C, double u)
{
    return -1. / C * log(u);
}

int singleNeutronTransport(const NeutronTransportParam* params)
{
    double neutron_pos = 0;

    double u = drand();
    double L = calculateDistance(params->C, u);
    double D = drand() * M_PI;
    double absorbedValue = 0;

    neutron_pos += L * cos(D);

    while (true)
    {
        absorbedValue = drand() * params->C;
        if (neutron_pos <= 0 || absorbedValue <= params->Cc)
        {
            return 0;
        }
        if (neutron_pos >= params->H)
        {
            return 1;
        }

        u = drand();
        L = calculateDistance(params->C, u);
        D = drand() * M_PI;
        neutron_pos += L * cos(D);
    }
}

MPI_Datatype getMpiNeutronTransportParam()
{
    int block_lengths[] = { 1, 1, 1, 1, 1 };
    MPI_Datatype types[] = { MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_LONG };
    MPI_Aint offsets[] = {
        offsetof(struct NeutronTransportParam, Cc),
        offsetof(struct NeutronTransportParam, Cs),
        offsetof(struct NeutronTransportParam, C),
        offsetof(struct NeutronTransportParam, H),
        offsetof(struct NeutronTransportParam, N)
    };

    MPI_Datatype mpi_neutron_transport_param_type;
    int err_code = MPI_Type_create_struct(5, block_lengths, offsets, types, &mpi_neutron_transport_param_type);
    if (err_code != MPI_SUCCESS) 
    {
        MPI_Abort(MPI_COMM_WORLD, err_code);
    }
    MPI_Type_commit(&mpi_neutron_transport_param_type);

    return mpi_neutron_transport_param_type;
}

