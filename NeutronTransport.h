#pragma once
#include <stdio.h>
#include <mpi.h>

struct NeutronTransportParam
{
    double Cc;
    double Cs;
    double C;
    double H;
    long N;
};

int initNeutronTransportParam(int argc, char** argv, NeutronTransportParam* params);

MPI_Datatype getMpiNeutronTransportParam();

int singleNeutronTransport(const NeutronTransportParam* );


