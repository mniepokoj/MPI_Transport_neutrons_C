#include "NeutronTransport.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <sprng.h>
#include <sprng_cpp.h>	

#define M_PI 3.141592653589793


int initNeutronTransportParam(int argc, char** argv, NeutronTransportParam* params)
{
    if (argc < 5)
    {
        return -1;
    }

    sscanf(argv[1], "%u", &params->N);
    sscanf(argv[2], "%lf", &params->Cc);
    sscanf(argv[3], "%lf", &params->Cs);
    sscanf(argv[4], "%lf", &params->H);
    params->C = params->Cc + params->Cs;

    return 1;
}


double calculateDistance(double C, double u)
{
    return -1.0 / C * log(u);
}

Result singleNeutronTransport(const NeutronTransportParam* params, Sprng *stream)
{
    double neutron_pos = 0;
    double u = stream->sprng();
    double L = calculateDistance(params->C, u);
    double D = stream->sprng() * M_PI;
    double capturedValue = 0;

    neutron_pos += L * cos(D);

    while (true)
    {
        capturedValue = stream->sprng() * params->C;
        if(capturedValue <= params->Cc)
        {
            return Result::Captured;
        }
        if (neutron_pos < 0)
        {
            return Result::Reflected;
        }
        else if(neutron_pos > params->H)
        {
            return Result::Passed;
        }

        u = stream->sprng();
        L = calculateDistance(params->C, u);
        D = stream->sprng() * M_PI;
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


void writeResultDataToFile(const NeutronTransportParam* params, const char* filename,
                            double chanceCapture, double chancePass, double chanceReflect, unsigned totalIterations)
{
    FILE *fp;
    fp = fopen(filename, "w");

    if (fp == NULL) 
    { 
        printf("Unable to write results to file");
    }

    fprintf(fp, "Solving the problem of transport to neutrons using the monte carlo method\n\n");
    fprintf(fp, "Cc=\t%lf\n", params->Cc );
    fprintf(fp, "Cs=\t%lf\n", params->Cs );
    fprintf(fp, "C=\t%lf\n", params->C );
    fprintf(fp, "H=\t%lf\n\n", params->H );
    fprintf(fp, "Number of iterations: %d\n", totalIterations);
    fprintf(fp, "Chance to capture neutron: %.23lf\n", chanceCapture);
    fprintf(fp, "Chance to neutron pass: %.23lf\n", chancePass);
    fprintf(fp, "Chance to reflect neutron: %.23lf\n", chanceReflect);
    printf("Data has been written to file '%s'\n", filename);

   fclose(fp);
}