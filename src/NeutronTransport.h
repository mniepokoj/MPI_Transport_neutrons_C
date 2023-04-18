#pragma once
#include <stdio.h>
#include <mpi.h>
#include <sprng.h>
#include "sprng_cpp.h"	

enum Result
{
    Captured,
    Passed,
    Reflected
};

struct NeutronTransportParam
{
    double Cc;
    double Cs;
    double C;
    double H;
    unsigned N;
};

int initNeutronTransportParam(int argc, char** argv, NeutronTransportParam* params);

MPI_Datatype getMpiNeutronTransportParam();

Result singleNeutronTransport(const NeutronTransportParam* , Sprng *stream);

void writeResultDataToFile(const NeutronTransportParam* params, const char* filename,
                            double chanceCapture, double chancePass, double chanceReflect, unsigned totalIterations);

